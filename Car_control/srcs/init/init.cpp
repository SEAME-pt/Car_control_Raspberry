#include "carControl.h"

// Initialize all core components required for correct program operation
t_carControl	initCarControl(int argc, char *argv[]) {

	t_carControl	carControl;

	//Initializing carControl with default values
	carControl.controller 		= nullptr;
	carControl.can				= nullptr;
	carControl.canInterface		= "can0";
	carControl.manual			= true;
	carControl.exit				= false;

	// Overriding default values using user input
	if (parsingArgv(argc, argv, &carControl) <= 0)
		return (carControl);

	// CAN_fd init
	try {
		carControl.can = init_can(carControl.canInterface);
	} catch (const CANController::CANException& e) {
		std::cerr << e.what() << std::endl;
		carControl.exit = true;
		return (carControl);
	} catch (...) {
		std::cerr << "Unexpected error during CAN init" << std::endl;
		carControl.exit = true;
		return (carControl);
	}

	// Joystick init
	//try {
	//	if (carControl.manual)
	//		carControl.controller = std::make_unique<Joystick>();
	//} catch (const std::exception& e) {
	//	std::cerr << e.what() << std::endl;
	//	carControl.exit = true;
	//	return (carControl);
	//}

	// Defer Qt application and communication initialization to the Qt thread
	carControl.app = nullptr;
	carControl.comm = nullptr;
	return (carControl);
}
