#include "../include/carControl.h"

// Condition for the main loop to keep running
bool	g_running = true;

static void	signalHandler(int signum) {

	(void)signum;
	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	signal(SIGINT, signalHandler);

	//inicialization of every crucial variables
	t_carControl carControl = initCarControl(argc, argv);

	//check if help input was requested
	if (carControl.helperMessage)
		return (0);

	std::cout << "🚗 Starting car control system...\n" << std::endl;

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
		std::cout << "Car stopped safely" << std::endl;

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

/*

# Intructions to run the program

# Default values
# Notice that default values are prepared only for the final result of comunication
# with STM32, properlly i2c connection and usage of joystick

cd build
make
./car

# For adicional input information:
./car --help

# Testing purposes inside coding machine

sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 mtu 72
sudo ip link set up vcan0

cd build
make

# Terminal 1: Monitor
candump vcan0

# Terminal 2: Run
sudo ./car --can=vcan0 --joy=false --i2c=false --debug

*/
