#include "carControl.h"

// Condition for the main loop to keep running
volatile bool	g_running = true;

static void	signalHandler(int signum) {

	(void)signum;
	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	try {
		t_carControl carControl = initCarControl(argc, argv);
		if (carControl.exit)
			return (1);
		carControl.controller = new joyStick(static_cast<const char *>("/dev/input/event6"));
		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);
		signal(SIGINT, signalHandler);
		while (g_running && !carControl.exit) {
			int16_t idk = carControl.controller->readPress();
			if (idk >= 0) {
				CANProtocol::sendDriveCommand(*carControl.can, 
					idk,1);
			}
		}
		if (carControl.controller)
			delete carControl.controller;
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	return (0);
}
