#include "carControl.h"

// Core loop to agregate joystick outputs and send them via CAN to the MCU
void	manualLoop(t_carControl *carControl) {

	int16_t last_steering	= 0;
	int16_t last_throttle 	= 0;

	while (g_running.load() && carControl->controller) {

		int		value 		= carControl->controller->readPress();
		int16_t	steering	= carControl->controller->getAbs(ABS_Z);
		int16_t	throttle	= carControl->controller->getAbs(ABS_Y);

		// Joystick disconection error
		if (value == -2) {
			CANProtocol::sendEmergencyBrake(*carControl->can, true);
			continue ;
		}

		if (value == START_BUTTON) {

			auto t_before = std::chrono::high_resolution_clock::now();

    		CANProtocol::sendEmergencyBrake(*carControl->can, true);
    
    		auto t_after = std::chrono::high_resolution_clock::now();
    		auto latency = std::chrono::duration_cast<std::chrono::microseconds>(t_after - t_before).count();
    
    		FILE* log = fopen("../tests/latencyTest/emergencyBreakLatencyTest.log", "a");
    		if (log) {
        		fprintf(log, "LATENCY,%lld\n", (long long)latency);
        		fclose(log);
    		}
    
    		g_running.store(false);
    		continue;
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
