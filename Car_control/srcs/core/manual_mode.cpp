#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl) {

	int16_t last_steering = 0;
	int16_t last_throttle = 0;

	while (g_running.load() && carControl->controller) {

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(true);
		int16_t	throttle	= carControl->controller->getAbs(false);

		if (value == START_BUTTON) {
			std::cout << "Initiating graceful shutdown.." << std::endl;
			g_running.store(false);
		}

		stableValues(&steering, &throttle);

		if (steering == last_steering && throttle == last_throttle)
			continue ;

		last_steering = steering;
		last_throttle = throttle;

		std::cout << "Steering: " << steering << " | Throttle: " << throttle << std::endl;
		CANProtocol::sendDriveCommand(*carControl->can, steering, throttle);
		readCan(carControl->can);
	}
}
