#include "carControl.h"

// Temp function just to print receiving vaues for now
void	readCan(const std::unique_ptr<CANController> &can) {

	can_frame	rx;
	memset(&rx, 0, sizeof(can_frame));

	if (can->receiveFrame(&rx) == 0) {  // 0 = sucesso
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
