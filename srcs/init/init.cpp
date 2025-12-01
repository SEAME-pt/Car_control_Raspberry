#include "../../include/carControl.h"

// Last step of verification, mostly used for debug
static int	finalVerification(t_carControl *carControl) {

	if (carControl->useJoystick && !carControl->joystick) {
		std::cerr << "Error! Joystick was not properlly inicialized.";
		cleanExit(carControl->joystick);
		return (0);
	}
	if (!carControl->can) {
		std::cerr << "Error! CAN was not properlly inicialized..." << std::endl;
		return (0);
	}
	return (1);
}

// Initializes all essential components for proper program execution
t_carControl	initCarControl(int argc, char *argv[]) {

	t_carControl	carControl;

	//Inicializing carControll with default values
	carControl.joystick			= nullptr;
	carControl.canInterface		= "can0";
	carControl.useJoystick		= true;
	carControl.debug 			= false;
	carControl.helperMessage	= false;
	carControl.useI2c			= true;
	carControl.can				= nullptr;

	if (parsingArgv(argc, argv, &carControl) <= 0)
		return (carControl);

	try {
		if (carControl.useJoystick)
			carControl.joystick = initCar();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	try {
		if (carControl.useI2c)
			initI2c();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		cleanSDL(carControl.joystick);
		exit(EXIT_FAILURE);
	}

	try {
		carControl.can = init_can(carControl.canInterface);
	} catch (...) {
		cleanExit(carControl.joystick);
		exit(EXIT_FAILURE);
	}

	if (finalVerification(&carControl) <= 0) {
		std::cerr << "Something went really wrong...\n" 
				<< "Shutting down in an unsafe way.";
		exit(EXIT_FAILURE);
	}
	return (carControl);
}

void	cleanExit(SDL_Joystick* joystick) {

	if (joystick)
		SDL_JoystickClose(joystick);
	I2c::All_close();
	SDL_Quit();
}