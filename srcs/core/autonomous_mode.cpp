#include "carControl.h"

void	autonomousLoop(const t_carControl &carControl) {

	while (g_running.load() && !carControl.exit) {

		CANProtocol::sendEmergencyBrake(*carControl.can, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		can_frame rx;
		memset(&rx, 0, sizeof(can_frame));
		if (carControl.can->receiveFrame(&rx) == 0) {  // 0 = sucesso
			std::cout << "ID: 0x" << std::hex << rx.can_id 
					<< " DLC: " << std::dec << (int)rx.can_dlc 
					<< " Data: ";
		for (int i = 0; i < rx.can_dlc; i++) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') 
				<< (int)rx.data[i] << " ";
		}
		std::cout << std::dec << std::endl;
		}
	}
}
