#include "carControl.h"

// Initialize all core components required for correct program operation
t_carControl	initCarControl(int argc, char *argv[]) {

	t_carControl	carControl;

	//Initializing carControl with default values
	carControl.controller = nullptr;
	carControl.canInterface		= "can0";
	carControl.useJoystick		= true;
	carControl.debug 			= false;
	carControl.exit				= false;
	carControl.can				= nullptr;

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
	carControl.controller = new joyStick(static_cast<const char *>("/dev/input/by-id/usb-SHANWAN_Android_Gamepad-event-joystick"));
	return (carControl);
}
