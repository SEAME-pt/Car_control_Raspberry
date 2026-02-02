#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl) {

	int16_t last_steering = 0;
	int16_t last_throttle = 0;
	int16_t last_brake = 0;

	while (g_running.load() && carControl->controller) {

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(ABS_Z);
		int16_t	throttle	= carControl->controller->getAbs(ABS_Y);

		if (value == START_BUTTON) {
			std::cout << "Initiating graceful shutdown.." << std::endl;
			g_running.store(false);
		}

		stableValues(&steering, &throttle);

		if (steering != last_steering) {
			last_steering = steering;
			CANProtocol::sendSteeringCommand(*carControl->can, steering);
			std::cout << "Steering: " << steering << std::endl;
		} 
		if (throttle != last_throttle) {
			last_throttle = throttle;
			CANProtocol::sendThrottleCommand(*carControl->can, throttle);
			std::cout << "Throttle: " << throttle << std::endl;
		}
		readCan(carControl->can);
	}
}
