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
		int no_data_count = 0;
		while (g_running && !carControl.exit) {
			int16_t value = carControl.controller->readPress();
			if (value >= 0) {
				std::cout << "Button pressed: " << value << std::endl;
				CANProtocol::sendDriveCommand(*carControl.can, 
					value,1);
				no_data_count = 0; // Reset counter when we get data
			} else {
				no_data_count++;
				if (no_data_count % 1000 == 0) {
					std::cout << "No joystick data... (checked " << no_data_count << " times)" << std::endl;
				}
			}
			// Small delay to prevent excessive CPU usage
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		if (carControl.controller)
			delete carControl.controller;
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	return (0);
}
