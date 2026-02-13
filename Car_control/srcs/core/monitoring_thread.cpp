#include "carControl.h"

// Global variable to track STM32 connection status
std::atomic<bool> g_stm32ConnectionLost(false);

void monitoringThread(t_CANReceiver* receiver) {

	constexpr auto STM32_TIMEOUT = std::chrono::milliseconds(600);

	auto lastSpeedDataReceived	= std::chrono::steady_clock::now();
	bool stm32Alive				= false;
	bool firstSpeedReceived		= false;

	while (g_running.load()) {
		try {
			auto now = std::chrono::steady_clock::now();

			// Check if received speed data (stm heartbeat)
			t_speedData speedData;
			if (getSpeedData(receiver, &speedData)) {
    			lastSpeedDataReceived = now;
    			std::cout << "[MONITORING] Speed: " 
					<< speedData.speedMps << " m/s (RPM: " 
					<< speedData.rpm << ")\n";

				if (!firstSpeedReceived) {
					firstSpeedReceived = true;
					stm32Alive = true;
					g_stm32ConnectionLost.store(false);
					std::cout << "[MONITORING] Connection stablished...\n";
				}
			}

			if (firstSpeedReceived) {
				auto timeSinceLastSpeed = now - lastSpeedDataReceived;

				if (timeSinceLastSpeed >= STM32_TIMEOUT) {
					if (stm32Alive) {
						stm32Alive = false;
						g_stm32ConnectionLost.store(true);
						std::cerr << "[MONITORING] STM32 connection lost! No speed data for: "
						<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastSpeed).count()
						<< "ms" << std::endl;
						CANProtocol::sendEmergencyBrake(*receiver->can, true);
					}
				} else if (!stm32Alive) {
					stm32Alive = true;
					std::cout << "[MONITORING] STM32 Connection restored!\n";
				}
			}
		} catch (const std::exception &e) {
			std::cerr << "[MONITORING] ERROR: " << e.what() << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
