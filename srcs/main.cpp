#include "../include/carControl.h"

// define the global bool to properly clear after sigint
bool	g_running = true;

static void	signalHandler(int signum) {

    std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	SDL_Joystick*	joystick		= nullptr;
	std::string		canInterface	= "can0";
	bool			useJoystick		= true;
	bool			debug 			= false;

	signal(SIGINT, signalHandler);

	//Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
		if (arg.find("--joy=") == 0) {
			std::string value = arg.substr(6);
            useJoystick = (value == "1" || value == "true" || value == "yes");
		}
        if (arg.find("--can=") == 0)
            canInterface = arg.substr(6);
        else if (arg == "--debug") {
            debug = true;
        } else if (arg == "--help" || arg == "-h") {
           std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "  --joy=0|1         Enable joystick (default: 1)\n"
                      << "  --can=INTERFACE   CAN interface (default: can0)\n"
                      << "  --debug           Enable debug output\n"
                      << "  --help            Show this help\n" << std::endl;
            return 0;
        }
    }

	//init of everything
	joystick = initCarControl(canInterface);
	if (!joystick) {
		std::cerr << "Cannot run without joystick" << std::endl;
		return (1);
	}
	auto can = init_can(canInterface);
	if (!can) {
        std::cerr << "Cannot run without CAN" << std::endl;
        cleanExit(joystick);
        return (1);
    }

	try {
		// Set initial state
        CANProtocol::sendDriveMode(*can, DriveMode::MANUAL);
        CANProtocol::sendEmergencyBrake(*can, false);
        CANProtocol::sendDriveCommand(*can, MID_ANGLE, 0);  // Center, stopped

		SDL_Event e;
		int frame_count = 0;

		while (g_running) {

			if (!joystick || !SDL_JoystickGetAttached(joystick)) {
				std::cerr << "Joystick disconnected!" << std::endl;
				g_running = false;
				break ;
			}

			CANProtocol::sendDriveCommand(*can, 
					joystickSteering(joystick), joystickThrottle(joystick));

			// Debug output
            if (debug && (frame_count % 25 == 0)) {
                std::cout << "🎮 S:" << (int)joystickSteering(joystick) 
                          << " T:" << (int)joystickThrottle(joystick) 
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
	CANProtocol::sendEmergencyBrake(*can, true);
	CANProtocol::sendDriveCommand(*can, 60, 0);
	std::cout << "ar stopped safely" << std::endl;

	} catch (CANController::CANException& e) {
		std::cerr << "Error: " << e.what() << std::endl;

		//try emergency stop even on error
		try {
			if (can) {
                CANProtocol::sendEmergencyBrake(*can, true);
                CANProtocol::sendDriveCommand(*can, MID_ANGLE, 0);
            }
		} catch (...) {
			std::cerr << "Failed to send emergency stop ";
			std::cerr << "prepare airbaigs..." << std::endl;
		}
	}
	cleanExit(joystick);
	return (0);
}
// git submodule update --init --recursive