#include "../../include/carControl.h"

// Final verification step, mainly used for debugging
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

// Initialize all core components required for correct program operation
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

	// Overriding default values using user input
	if (parsingArgv(argc, argv, &carControl) <= 0)
		return (carControl);

	// Joystick init
	try {
		if (carControl.useJoystick)
			carControl.joystick = initCar();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	// I2c init
	try {
		if (carControl.useI2c)
			initI2c();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		cleanSDL(carControl.joystick);
		exit(EXIT_FAILURE);
	}

	// CAN_fd init
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