#include "carControl.h"

SDL_Joystick*	initJoystick() {

	// Initialize SDL joystick subsystem
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
		throw (InitException("Failed to initialize SDL joystick"));

	// Check how many joysticks are connected
	if (SDL_NumJoysticks() <= 0) {
		SDL_Quit();
		throw (InitException("No joystick detected"));
	}

	// Open the first available joystick (index 0)
	SDL_Joystick *joystick = SDL_JoystickOpen(CONTROLLER_0);
	if (joystick == nullptr) {
		SDL_Quit();
		throw (InitException("Failed to open joystick"));
	}

	std::cout << "Joystick detected: " << SDL_JoystickName(joystick) << std::endl;
	return (joystick);
}
