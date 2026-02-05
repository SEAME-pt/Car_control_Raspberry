#include "carControl.h"

void heartbeatThread(CANController* can) {

	std::cout << "Heartbeat thread started" << std::endl;

	while (g_running.load()) {
		try {
			CANProtocol::sendHeartbeat(*can);
		} catch (const std::exception& e) {
            std::cerr << "Heartbeat error: " << e.what() << std::endl;
        }
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
