#include "carControl.h"

void monitoringThread(t_CANReceiver* receiver) {

	auto lastSpeedDataReceived	= std::chrono::steady_clock::now();

	bool stm32Alive				= false;
	bool firstSpeedReceived		= false;

	constexpr auto STM32_TIMEOUT = std::chrono::milliseconds(600);

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
					std::cout << "Connection stablished! Monitoring...\n";
				}
			}

			if (firstSpeedReceived) {
				auto timeSinceLastSpeed = now - lastSpeedDataReceived;

				if (timeSinceLastSpeed >= STM32_TIMEOUT) {
					if (stm32Alive) {
						stm32Alive = false;
						std::cerr << "WARNING: STM32 connection lost! No speed data for: "
						<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastSpeed).count()
						<< "ms" << std::endl;
						CANProtocol::sendEmergencyBrake(*receiver->can, true);
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
