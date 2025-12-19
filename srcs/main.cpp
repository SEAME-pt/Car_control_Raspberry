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
		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);
		signal(SIGINT, signalHandler);
		std::cout << "Starting joystick reading loop..." << std::endl;
		while (g_running && !carControl.exit) {
			int16_t value = carControl.controller->readPress();
			int16_t steering = carControl.controller->getAbs(true);
			int16_t throttle = carControl.controller->getAbs(false);
			if (value >= 0) {
				std::cout << "Button pressed: " << value << std::endl;
			}
			std::cout << "Steering: " << steering << " | Throttle: " << throttle << std::endl;
			int8_t steering_cmd = static_cast<int8_t>(((steering + 127) * 120) / 254);
			int8_t throttle_cmd = static_cast<int8_t>((throttle * 100) / 127);
			CANProtocol::sendDriveCommand(*carControl.can, steering_cmd, throttle_cmd);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		if (carControl.controller)
			delete carControl.controller;
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	return (0);
}
