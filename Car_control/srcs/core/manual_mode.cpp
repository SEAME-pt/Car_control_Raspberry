#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl) {

	int16_t last_steering	= 0;
	int16_t last_throttle 	= 0;

	while (g_running.load() && carControl->controller) {

		std::this_thread::sleep_for(std::chrono::milliseconds(7));
		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(ABS_Z);
		int16_t	throttle	= carControl->controller->getAbs(ABS_Y);

		// Joystick disconection error
		if (value == -2) {
			CANProtocol::sendEmergencyBrake(*carControl->can, true);
			continue ;
		}

		if (value == START_BUTTON) {
			auto t_start = std::chrono::high_resolution_clock::now();
    		auto micros = std::chrono::duration_cast<std::chrono::microseconds>(t_start.time_since_epoch()).count();

    		// Log to file
    		FILE* log = fopen("latency_test.log1", "a");
    		if (log) {
        		fprintf(log, "BUTTON_PRESS,%lld\n", (long long)micros);
        		fclose(log);
    		} else
				std::cout << "Not possible" << std::endl;

    		std::cout << "Initiating graceful shutdown..." << std::endl;
    		g_running.store(false);
		} else if (value == A_BUTTON) {
			CANProtocol::sendEmergencyBrake(*carControl->can, true);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue ;
		}

		stableValues(&steering, &throttle);

		if (steering != last_steering || throttle != last_throttle) {
			CANProtocol::sendDrivingCommand(*carControl->can, throttle, steering);
			std::cout << "Throttle: " << throttle << " | Steering: " << steering << std::endl;
			last_steering = steering;
			last_throttle = throttle;
		}
	}
}
