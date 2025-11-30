#include "../../include/carControl.h"

SDL_Joystick*	initCarControl(const std::string &canInterface) {

	SDL_Joystick *joystick = nullptr;
	try {
		joystick = initCar();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exitSDL(joystick);
	}

	try {
		initI2c();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return (joystick);
}

void	cleanExit(SDL_Joystick* joystick) {

	if (joystick) {
		SDL_JoystickClose(joystick);
		joystick = nullptr;
	}

	I2c::All_close();
    SDL_Quit();
	exit(EXIT_SUCCESS);
}