#include "../include/carControl.h"

// define the global joystick instance to properly clear after sigint
SDL_Joystick*		g_joystick = nullptr;
bool				g_running = true;

static void	cleanExit() {

	if (g_joystick) {
		SDL_JoystickClose(g_joystick);
		g_joystick = nullptr;
	}

	I2c::All_close();
    SDL_Quit();
	cleanup_can();
	exit(EXIT_SUCCESS);
}

static void	signalHandler(int signum) {

    std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
	cleanExit();
}

int main() {

	SDL_Joystick *joystick = NULL;
	std::string can_interface = "can0";
	bool debug = false;

	signal(SIGINT, signalHandler);

	try {
		joystick = initCar();
		g_joystick = joystick;
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exitSDL();
		return (1);
	}

	try {
		initI2c();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	if (!init_can(can_interface)) {
        std::cerr << "❌ Cannot run without CAN" << std::endl;
        cleanExit();
        return 1;
    }

	auto* can = get_can();

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

			float axisSteering = SDL_JoystickGetAxis(joystick, 2) / MAX_AXIS_VALUE;
			float axisThrottle = SDL_JoystickGetAxis(joystick, 1) / MAX_AXIS_VALUE;

			uint8_t steering = static_cast<uint8_t>(
                std::clamp(axisSteering * 60.0f + 60.0f, 0.0f, 120.0f)
            );
            int8_t throttle = static_cast<int8_t>(
                std::clamp(-axisThrottle * 100.0f, -100.0f, 100.0f)
            );

			CANProtocol::sendDriveCommand(*can, steering, throttle);

			// Debug output
            if (debug && (frame_count % 25 == 0)) {
                std::cout << "🎮 S:" << (int)steering 
                          << " T:" << (int)throttle 
                          << " [" << frame_count << "]" << std::endl;
            }

			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_JOYBUTTONDOWN) {
					if (e.jbutton.button == START_BUTTON) {
						std::cout << "START button pressed. Exiting..." << std::endl;
						CANProtocol::sendEmergencyBrake(*can, true);
						CANProtocol::sendDriveCommand(*can, 60, 0);
						g_running = false;
						break ;
					}
				}
			}
			frame_count++;
			usleep(20000);  // 50Hz
		}
	} catch (CANController::CANException& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	cleanExit();
	return (0);
}
// git submodule update --init --recursive