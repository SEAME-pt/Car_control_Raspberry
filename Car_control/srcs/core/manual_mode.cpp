#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl, t_CANReceiver* receiver) {

    int16_t last_steering	= 0;
    int16_t last_throttle 	= 0;

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

        if (steering != last_steering || throttle != last_throttle) {
            CANProtocol::sendDrivingCommand(*carControl->can, throttle, steering);
            std::cout << "Throttle: " << throttle << " | Steering: " << steering << std::endl;
            last_steering = steering;
            last_throttle = throttle;
        }

        t_speedData speedData;
        if (getSpeedData(receiver, &speedData)) {
            std::cout << "Speed: " << speedData.speedMps << " m/s (RPM: " 
                      << speedData.rpm << ")" << std::endl;
        }
    }
}
