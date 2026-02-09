#include "carControl.h"
#include <memory>

int	main(int argc, char *argv[]) {

	signalManager();

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);
	try {
		if (!carControl.manual) {
			std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
			manualLoop(&carControl); //without input_event struct too nothing, bare everything
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	try {
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
	} catch (...) {
		return (1);
	}
	return (0);
}
