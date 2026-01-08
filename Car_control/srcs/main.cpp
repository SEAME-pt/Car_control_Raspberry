#include "carControl.h"

// Condition for the main loop to keep running
std::atomic<bool> g_running{true};

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
			manualLoop(&carControl);
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	return (0);
}
