#include "carControl.h"

// Core loop to agregate ai and autonomous automotive architecture
void	autonomousLoop(const t_carControl &carControl) {

	while (g_running.load() && !carControl.exit) {

		CANProtocol::sendEmergencyBrake(*carControl.can, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(1200));

		readCan(carControl.can);
	}
}
