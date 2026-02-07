#include "carControl.h"

void monitoringThread(t_CANReceiver* receiver) {
    
	auto lastHeartbeatSent		= std::chrono::steady_clock::now();
	auto lastSpeedDataReceived	= std::chrono::steady_clock::now();

	bool stm32Alive				= false;
	bool firstSpeedReceived		= false;

	constexpr auto HEARTBEAT_INTERVAL	= std::chrono::milliseconds(1000);
	constexpr auto STM32_TIMEOUT		= std::chrono::milliseconds(2000);

	while (g_running.load()) {
		try {
			auto now = std::chrono::steady_clock::now();

			// Send heartbeat every 500ms
			if (now - lastHeartbeatSent >= HEARTBEAT_INTERVAL) {
				CANProtocol::sendEmergencyBrake(*receiver->can, false);
				lastHeartbeatSent = now;
			}
			// Check if received speed data (stm heartbeat)
			{
				std::lock_guard<std::mutex>	lock(receiver->speedMutex);
				if (!receiver->speedQueue.empty()) {
					lastSpeedDataReceived = now;

					if (!firstSpeedReceived) {
						firstSpeedReceived = true;
						stm32Alive = true;
						std::cout << "Connection stablished! Monitoring...\n";
					}
				} /* else {
					// If speed data received, print it
					t_speedData speedData;
        			if (getSpeedData(receiver, &speedData)) {
            			std::cout << "Speed: " << speedData.speedMps << " m/s (RPM: " 
                      				<< speedData.rpm << ")" << std::endl;
        			}
				} */
			}

			if (firstSpeedReceived) {
				auto timeSinceLastSpeed = now - lastSpeedDataReceived;

				if (timeSinceLastSpeed >= STM32_TIMEOUT) {
					if (stm32Alive) {
						stm32Alive = false;
						std::cerr << "WARNING: STM32 connection lost! No speed data for: "
						<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastSpeed).count()
						<< "ms" << std::endl;
					}
				} else if (!stm32Alive) {
					stm32Alive = true;
					std::cout << "STM32 Connection restored!\n";
				}
			}
		} catch (const std::exception &e) {
			std::cerr << "[MONITORING] ERROR: " << e.what() << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
