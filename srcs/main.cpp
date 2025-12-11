#include "carControl.h"

// Condition for the main loop to keep running
volatile bool	g_running = true;

static void	signalHandler(int signum) {

	(void)signum;
	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {
	const char *device = "/dev/input/event25";

	try {
		t_carControl carControl = initCarControl(argc, argv);
		carControl.controller = new joyStick(static_cast<const char *>("/dev/input/event25"));
		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);  // Center, stopped
		signal(SIGINT, signalHandler);
		while (g_running) {
			__u16 idk = carControl.controller->readPress();
			if (idk != 0) {
				CANProtocol::sendDriveCommand(*carControl.can, 
					idk,1);
			}
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	return (0);
}
