#include "carControl.h"

// Temp function just to print receiving vaues for now
void	readCan(const std::unique_ptr<CANController> &can) {
    can_frame	rx;
    memset(&rx, 0, sizeof(can_frame));

    if (can->receiveFrame(&rx) == 0) {  // 0 = sucesso
        if (rx.can_dlc >= 2) {
            uint16_t rpm = (rx.data[0] << 8) | rx.data[1];
            std::cout << "Raw RPM: " << rpm << std::endl;
        }
    }
}
