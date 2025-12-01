#include "../include/carControl.h"

// define the global bool to properly clear after sigint
bool	g_running = true;

static void	signalHandler(int signum) {

	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	t_carControl	carControl;

	signal(SIGINT, signalHandler);

	//inicialization of every crucial variables
	carControl = initCarControl(argc, argv);
	if (carControl.helperMessage)
		return (0);

	try {
		// Set initial state
		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);  // Center, stopped

		SDL_Event e;
		int frame_count = 0;

		while (g_running) {

			if (!carControl.joystick || !SDL_JoystickGetAttached(carControl.joystick)) {
				std::cerr << "Joystick disconnected!" << std::endl;
				g_running = false;
				break ;
			}

			CANProtocol::sendDriveCommand(*carControl.can, 
					joystickSteering(carControl.joystick), 
					joystickThrottle(carControl.joystick));

			// Debug output
			if (carControl.debug && (frame_count % 25 == 0)) {
				std::cout << "🎮 S:" << (int)joystickSteering(carControl.joystick) 
						  << " T:" << (int)joystickThrottle(carControl.joystick) 
						  << " [" << frame_count << "]" << std::endl;
			}

			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_JOYBUTTONDOWN) {
					if (e.jbutton.button == START_BUTTON) {
						std::cout << "START button pressed. Exiting..." << std::endl;
						g_running = false;
						break ;
					}
				}
			}
			frame_count++;
			usleep(20000);  // 50Hz
		}
	std::cout << "Stopping car..." << std::endl;
	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	CANProtocol::sendDriveCommand(*carControl.can, 60, 0);
	std::cout << "ar stopped safely" << std::endl;

	} catch (CANController::CANException& e) {
		std::cerr << "Error: " << e.what() << std::endl;

		//try emergency stop even on error
		try {
			if (carControl.can) {
				CANProtocol::sendEmergencyBrake(*carControl.can, true);
				CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);
			}
		} catch (...) {
			std::cerr << "Failed to send emergency stop ";
			std::cerr << "prepare airbaigs..." << std::endl;
		}
	}
	cleanExit(carControl.joystick);
	return (0);
}
// git submodule update --init --recursive