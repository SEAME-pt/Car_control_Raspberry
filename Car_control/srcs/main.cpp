#include "carControl.h"

int	main(int argc, char *argv[]) {

	signalManager();

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);

	std::thread hbThread(heartbeatThread, carControl.can.get());

	try {
		if (!carControl.manual) {
			std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
			manualLoop(&carControl);
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	g_running.store(false);
	if (hbThread.joinable())
		hbThread.join();

	try {
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
	} catch (...) {
		return (1);
	}
	return (0);
}
