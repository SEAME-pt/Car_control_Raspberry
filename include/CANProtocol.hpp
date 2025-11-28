#pragma once

#include "CANController.hpp"
#include <cstdint>

namespace CANID {
    constexpr uint32_t EMERGENCY_BRAKE = 0x100;	//max priority
    constexpr uint32_t DRIVE_COMMAND   = 0x101;	//high priority
    constexpr uint32_t DRIVE_MODE      = 0x102;	//low priority
};

enum class DriveMode : uint8_t {
    MANUAL 		= 0x00,
    AUTONOMOUS	= 0x01
};

namespace CANProtocol {

	inline void sendEmergencyBrake(CANController& can, bool active) {
        uint8_t data[1] = { active ? 0xFF : 0x00 };
        can.sendFrameFD(CANID::EMERGENCY_BRAKE, data, 1);
    }

	inline void sendDriveCommand(CANController& can, int16_t throttle, int16_t steering) {
        
		uint8_t data[4];

		data[0] = (throttle >> 8) & 0xFF;
        data[1] = throttle & 0xFF;
        data[2] = (steering >> 8) & 0xFF;
        data[3] = steering & 0xFF;

		can.sendFrameFD(CANID::DRIVE_COMMAND, data, 4);
	}

	inline void sendDriveMode(CANController& can, DriveMode mode) {
        uint8_t data[1] = { static_cast<uint8_t>(mode) };
        can.sendFrameFD(CANID::DRIVE_MODE, data, 1);
    }
}