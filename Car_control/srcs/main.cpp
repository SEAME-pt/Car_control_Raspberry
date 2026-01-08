#include "carControl.h"

// Condition for the main loop to keep running
std::atomic<bool> g_running{true};

int main(int argc, char *argv[]) {

	signalManager();

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);

	try {
		if (!carControl.useJoystick) {
			std::cout << "Joystick not detected, running in debug mode." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Starting Joystick reading loop..." << std::endl;
			manualLoop(&carControl);
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	return (0);
}
