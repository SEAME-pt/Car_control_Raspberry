#include "carControl.h"

void	canReceiverThread(t_CANReceiver* receiver) {

	can_frame	rx;

	while (g_running.load()) {

		memset(&rx, 0, sizeof(can_frame));
		if (receiver->can->receiveFrame(&rx) == 0) {

			auto now = std::chrono::steady_clock::now();

			switch (rx.can_id) {

				// Speed sensor
				case CANRECEIVERID::SPEEDRPMSTM32: {
					if (rx.can_dlc >= 2) {
						t_speedData speedData;
						speedData.rpm = (rx.data[0] << 8) | rx.data[1];

						std::lock_guard<std::mutex> lock(receiver->speedMutex);
						receiver->speedQueue.push(speedData);

						// Limit the size to a max of only 10 entries
						if (receiver->speedQueue.size() > 10)
							receiver->speedQueue.pop();
					}
				}
				break ;

				// Battery status
				case CANRECEIVERID::BATTERYSTM32: {
					if (rx.can_dlc >= 4) {
						t_batteryData	batteryData;
						batteryData.percentage = (rx.data[0] << 8) | rx.data[1];
						batteryData.voltage = (rx.data[2] << 8) | rx.data[3];

						std::lock_guard<std::mutex> lock(receiver->batteryMutex);
						receiver->batteryQueue.push(batteryData);

						if (receiver->heartbeatQueue.size() > 5)
							receiver->speedQueue.pop();
					}
				}
				break ;

				// Heartbeat ACK
                case CANRECEIVERID::HEARTBEAT_STM: {
                    if (rx.can_dlc >= 1) {
                        t_heartbeatData hbData;
                        hbData.ack = rx.data[0];
                        hbData.timestamp = now;

                        std::lock_guard<std::mutex> lock(receiver->heartbeatMutex);
                        receiver->heartbeatQueue.push(hbData);

                        if (receiver->heartbeatQueue.size() > 1) {
                            receiver->heartbeatQueue.pop();
                        }
                    }
                    break ;

					default:
                    std::cout << "Unknown CAN ID: 0x" << std::hex 
                              << rx.can_id << std::dec << std::endl;
                    break ;
                }
			}
			// Small sleep to avoid burning CPU
        	std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}
