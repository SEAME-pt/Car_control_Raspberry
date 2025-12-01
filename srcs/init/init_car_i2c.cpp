#include "../../include/carControl.h"

void	initI2c() {

	try {
        I2c::All_init();
        I2c::set_servo_angle(MID_ANGLE);
    } catch (const std::exception &e) {
        throw (InitException("ERROR! I2C initialization failed."));
    }
}

SDL_Joystick*	initCar() {
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
		throw (InitException("Failed to initialize SDL joystick"));

	if (SDL_NumJoysticks() <= 0) {
		SDL_Quit();
		throw (InitException("No joystick detected"));
	}

	SDL_Joystick *joystick = SDL_JoystickOpen(CONTROLLER_0);
	if (joystick == nullptr) {
		SDL_Quit();
		throw (InitException("Failed to open joystick"));
	}

	std::cout << "Joystick detected: " << SDL_JoystickName(joystick) << std::endl;
	return (joystick);
}

void	cleanSDL(SDL_Joystick* joystick) {

	if (joystick)
		SDL_JoystickClose(joystick);
}
