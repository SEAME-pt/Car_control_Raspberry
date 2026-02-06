#include "carControl.h"

void heartbeatThread(t_CANReceiver* receiver) {
    
    auto lastHeartbeatAck	= std::chrono::steady_clock::now();
    bool stm32Alive 		= false;
	bool firstAckReceived	= false;

    while (g_running.load()) {
        try {
            // Send heartbeat to STM32
            CANProtocol::sendHeartbeat(*receiver->can);
            
            // Check for heartbeat ACK in queue
            t_heartbeatData hbData;
            if (getHeartbeatAck(receiver, &hbData)) {
                lastHeartbeatAck = hbData.timestamp;
                if (!firstAckReceived) {
                    std::cout << "STM32 connected! Heartbeat monitoring active." << std::endl;
                    firstAckReceived = true;
                    stm32Alive = true;
                }
                
                if (!stm32Alive) {
                    std::cout << "STM32 heartbeat restored!" << std::endl;
                    stm32Alive = true;
                }
            }

            // Check timeout only after first ack
			if (firstAckReceived) {

				auto now = std::chrono::steady_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
					now - lastHeartbeatAck);
				
				// Timeout: 900ms without heartbeat ACK = STM32 dead
				if (elapsed.count() > 900) {
					if (stm32Alive) {
						std::cerr << "STM32 HEARTBEAT TIMEOUT! No ACK for " 
								  << elapsed.count() << "ms" << std::endl;
						std::cerr << "Triggering emergency brake!!!!!" << std::endl;
						
						stm32Alive = false;
						CANProtocol::sendEmergencyBrake(*receiver->can, true);
						g_running.store(false);  // Stop the program
					}
				}
			}

        } catch (const std::exception& e) {
            std::cerr << "Heartbeat error: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}
