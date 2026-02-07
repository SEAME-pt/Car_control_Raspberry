#include "carControl.h"
#include <memory>

int	main(int argc, char *argv[]) {

	signalManager();

	auto joystick = std::make_unique<Joystick>();

	// Joystick test: read axes and buttons. Press START to exit.
	while (g_running) {
		if (joystick->isDisconnected()) {
			std::cout << "\n⚠️  Controller disconnected! Exiting..." << std::endl;
			break;
		}
		
		int btn = joystick->readPress();
		if (btn != -1) {
			if (btn == START_BUTTON) {
				break;
			}
		}
		int16_t throttle = joystick->getAbs(ABS_Y);
		int16_t steering = joystick->getAbs(ABS_Z);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	std::cout << std::endl;

	//t_carControl carControl = initCarControl(argc, argv);
	//if (carControl.exit)
	//	return (1);

	//try {
	//	if (!carControl.manual) {
	//		std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
	//		autonomousLoop(carControl);
	//	} else {
	//		std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
	//		manualLoop(&carControl);without input_event struct too nothing, bare everything
	//	}
	//} catch (const std::exception &err) {
	//	std::cerr << err.what() << std::endl;
	//}
	//try {
	//	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	//} catch (...) {
	//	return (1);
	//}
	return (0);
}
