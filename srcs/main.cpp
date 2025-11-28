#include "../include/carControl.h"

// define the global joystick instance to properly clear after sigint
SDL_Joystick*		g_joystick = nullptr;
bool				g_running = true;

int main() {

	int steering = MID_ANGLE;		//rotation
	int throttle = 0;				//direction & speed
	SDL_Joystick *joystick = NULL;

	signal(SIGINT, signalHandler);

	try {
		joystick = initCar();
		g_joystick = joystick;
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exitSDL();
	}

	try {
		initI2c();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	SDL_Event e;
	while (g_running) {

		if (!joystick || !SDL_JoystickGetAttached(joystick)) {
			std::cerr << "Joystick disconnected!" << std::endl;
			g_running = false;
			break ;
		}

		float axisSteering = SDL_JoystickGetAxis(joystick, 2) / MAX_AXIS_VALUE;
		float axisThrottle = SDL_JoystickGetAxis(joystick, 1) / MAX_AXIS_VALUE;

		steering = static_cast<int>(mapAxisToAngle(axisSteering, 0, 120, 60));
		throttle = static_cast<int>(mapAxisToAngle(axisThrottle, -100, 100, 0));

		I2c::set_servo_angle(steering);

		if (throttle > 0)
			I2c::motor(0, throttle, 1); // Backward
		else if (throttle < 0)
			I2c::motor(0, -throttle, 0); // Forward
		else
			I2c::stop_motors(); // Stop

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_JOYBUTTONDOWN) {
				if (e.jbutton.button == START_BUTTON) {
					std::cout << "START button pressed. Exiting..." << std::endl;
					g_running = false;
					break ;
				}
			}
		}
		SDL_Delay(25);
	}

	cleanExit();
	return (0);
}

// git submodule update --init --recursive