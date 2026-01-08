#include "carControl.h"
#include <iomanip>

// Condition for the main loop to keep running
std::atomic<bool> g_running{true};

int main(int argc, char *argv[]) {

	signalManager();

	static int16_t last_steering = 0;
	static int16_t last_throttle = 0;

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);


	try {
		if (!carControl.useJoystick) {
			std::cout << "Joystick not detected, running in debug mode." << std::endl;
			while (g_running.load() && !carControl.exit) {

				CANProtocol::sendEmergencyBrake(*carControl.can, true);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				can_frame rx;
				memset(&rx, 0, sizeof(can_frame));
				if (carControl.can->receiveFrame(&rx) == 0) {  // 0 = sucesso
					std::cout << "ID: 0x" << std::hex << rx.can_id 
							  << " DLC: " << std::dec << (int)rx.can_dlc 
							 << " Data: ";
					for (int i = 0; i < rx.can_dlc; i++) {
						std::cout << std::hex << std::setw(2) << std::setfill('0') 
						  	<< (int)rx.data[i] << " ";
					}
					std::cout << std::dec << std::endl;
				}
			}
		} else {
			std::cout << "Starting Joystick reading loop..." << std::endl;
			while (g_running.load() && !carControl.exit && carControl.controller != nullptr) {

				int		value = carControl.controller->readPress();
				int16_t	steering = carControl.controller->getAbs(true);
				int16_t	throttle = carControl.controller->getAbs(false);

				if (value == START_BUTTON) {
					std::cout << "Initiating graceful shutdown.." << std::endl;
					g_running = false;
				}

				if (steering == last_steering && throttle == last_throttle)
					continue;

				last_steering = steering;
				last_throttle = throttle;

				int16_t steering_cmd = static_cast<int16_t>(((steering + 127) * 120) / 254);
				int16_t throttle_cmd = static_cast<int16_t>((throttle * 100) / 127);

				CANProtocol::sendDriveCommand(*carControl.can, steering_cmd, throttle_cmd);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

		}

	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	return (0);
}
