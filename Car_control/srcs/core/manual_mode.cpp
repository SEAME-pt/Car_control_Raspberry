#include "carControl.h"

void	manualLoop(t_carControl *carControl) {

	int16_t last_steering = 0;
	int16_t last_throttle = 0;

	while (g_running.load() && carControl->controller) {

		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(true);
		int16_t	throttle	= carControl->controller->getAbs(false);

		if (steering == -1 || throttle == -1) {
			throw std::runtime_error("ERROR! Impossible to read joystick input...");
		}

		if (value == START_BUTTON) {
			std::cout << "Initiating graceful shutdown.." << std::endl;
			g_running = false;
		}

		if (steering == last_steering && throttle == last_throttle)
			continue ;

		last_steering = steering;
		last_throttle = throttle;

		int16_t steering_cmd = static_cast<int16_t>(((steering + 127) * 120) / 254);
		int16_t throttle_cmd = static_cast<int16_t>((throttle * 100) / 127);

		CANProtocol::sendDriveCommand(*carControl->can, steering_cmd, throttle_cmd);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
