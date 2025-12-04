#include "carControl.h"

// Condition for the main loop to keep running
volatile bool	g_running = true;

static void	signalHandler(int signum) {

	(void)signum;
	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	signal(SIGINT, signalHandler);

	//initialization of all crucial variables
	t_carControl carControl = initCarControl(argc, argv);

	//check if help input was requested
	if (carControl.exit)
		return (1);

	std::cout << "Starting car control main loop...\n" << std::endl;

	try {
		// Set initial state
		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);  // Center, stopped

		SDL_Event e;
		int frame_count = 0;

		while (g_running) {

			// Continuous verification of joystick connection
			if (carControl.useJoystick) {
				if (!carControl.joystick || !SDL_JoystickGetAttached(carControl.joystick)) {
					std::cerr << "Joystick disconnected!" << std::endl;
					g_running = false;
					break ;
				}

				// Send joystick commands
				CANProtocol::sendDriveCommand(*carControl.can, 
					joystickSteering(carControl.joystick), 
					joystickThrottle(carControl.joystick));

				// Debug output
				if (carControl.debug && (frame_count % 25 == 0)) {
					std::cout << "🎮 S:" << (int)joystickSteering(carControl.joystick)
						  << " T:" << (int)joystickThrottle(carControl.joystick)
						  << " [" << frame_count << "]" << std::endl;
				}

				// Handle button events
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_JOYBUTTONDOWN) {
						if (e.jbutton.button == START_BUTTON) {
							std::cout << "START button pressed. Exiting..." << std::endl;
							g_running = false;
							break ;
						}
					}
				}
			} else {
				// No joystick mode - keep stopped
				// Future integration of autonomous mode
				CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);
			}
			frame_count++;
			usleep(20000);  // 50Hz
		}

		//emergency stop
		std::cout << "Stopping car..." << std::endl;
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
		CANProtocol::sendDriveCommand(*carControl.can, 60, 0);
		usleep(100000); // Wait 100ms for transmission
		std::cout << "Car stopped safely!" << std::endl;

	} catch (const CANController::CANException& e) {
		std::cerr << "Error: " << e.what() << std::endl;

		//try emergency stop again even on error
		try {
			if (carControl.can) {
				CANProtocol::sendEmergencyBrake(*carControl.can, true);
				CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);
			}
		} catch (...) {
			std::cerr << "Failed to send emergency stop... Prepare AIRBAGS!" << std::endl;
		}
	}
	cleanExit(carControl.joystick);
	return (0);
}

// git submodule update --init --recursive
