#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl, t_CANReceiver* receiver) {

	int16_t last_steering	= 0;
	int16_t last_throttle 	= 0;
	uint8_t	counter			= 0;

	while (g_running.load() && carControl->controller) {

		std::this_thread::sleep_for(std::chrono::milliseconds(7));
		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(ABS_Z);
		int16_t	throttle	= carControl->controller->getAbs(ABS_Y);

		if (value == START_BUTTON) {
			std::cout << "Initiating graceful shutdown.." << std::endl;
			g_running.store(false);
		}

		stableValues(&steering, &throttle);

		if (steering != last_steering) {
    		CANProtocol::sendSteeringCommand(*carControl->can, steering);
    		std::cout << "Steering: " << steering << std::endl;
    		last_steering = steering;
		}

		if (throttle == 0) {
    		if (counter < 3) {
        		CANProtocol::sendThrottleCommand(*carControl->can, throttle);
        		std::cout << "Throttle: " << throttle << std::endl;
        		counter++;
    		}
		} else if (throttle != last_throttle) {
    		CANProtocol::sendThrottleCommand(*carControl->can, throttle);
    		std::cout << "Throttle: " << throttle << std::endl;
    		counter = 0;
		}
		last_throttle = throttle;

		t_speedData speedData;
        if (getSpeedData(receiver, &speedData)) {
            std::cout << "Speed: " << speedData.speedMps << " m/s (RPM: " 
                      << speedData.rpm << ")" << std::endl;
        }
	}
}
