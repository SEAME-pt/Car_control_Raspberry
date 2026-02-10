#include "carControl.h"

extern CarDataCommunication *comm;

void	canReceiverThread(t_CANReceiver* receiver) {

	can_frame	rx;

	while (g_running.load()) {

		memset(&rx, 0, sizeof(can_frame));
		if (receiver->can->receiveFrame(&rx) == 0) {

			auto now = std::chrono::steady_clock::now();

			switch (rx.can_id) {

				// Speed sensor
				case CANRECEIVERID::SPEEDRPMSTM32: {
					if (rx.can_dlc >= 1) {
						std::cout << "Received SPEEDRPMSTM32 frame inside" << std::endl;
						t_speedData speedData;
						speedData.rpm = (rx.data[0] << 8) | rx.data[1];

						std::lock_guard<std::mutex> lock(receiver->speedMutex);
						receiver->speedQueue.push(speedData);
						std::cout << "Received RPM: " << speedData.rpm << std::endl;
						//comm->updateSpeed(speedData.rpm);
						//comm->sendData();
						// Limit the size to a max of only 10 entries
						if (receiver->speedQueue.size() > 10)
							receiver->speedQueue.pop();
					}
				}
				break ;

				// Battery status
				case CANRECEIVERID::BATTERYSTM32: {
					if (rx.can_dlc >= 3) {
						t_batteryData	batteryData;
						batteryData.percentage = (rx.data[0] << 8) | rx.data[1];
						batteryData.voltage = rx.data[2];

						std::lock_guard<std::mutex> lock(receiver->batteryMutex);
						receiver->batteryQueue.push(batteryData);
						std::cout << "Received Battery: " << (int) batteryData.percentage << "%, Voltage: " << batteryData.voltage << "V" << std::endl;


						if (receiver->batteryQueue.size() > 5)
							receiver->batteryQueue.pop();
					}
				}
				break ;

					default:
                    std::cout << "Unknown CAN ID: 0x" << std::hex 
                              << rx.can_id << std::dec << std::endl;
                    break ;
			}
        	std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}
