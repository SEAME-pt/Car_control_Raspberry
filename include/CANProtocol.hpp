#pragma once

#include "CANController.hpp"
#include <algorithm>

namespace CANID {
    constexpr uint32_t EMERGENCY_BRAKE = 0x100;	//max priority
    constexpr uint32_t DRIVE_COMMAND   = 0x101;	//high priority
    constexpr uint32_t DRIVE_MODE      = 0x102;	//low priority
};

enum class DriveMode : int8_t {
    MANUAL 		= 0x00,
    AUTONOMOUS	= 0x01
};

namespace CANProtocol {

	inline void sendEmergencyBrake(CANController& can, bool active) {
        int8_t data[1] = { active ? 0xFF : 0x00 };
        can.sendFrameFD(CANID::EMERGENCY_BRAKE, data, 1);
    }

	inline void sendDriveCommand(CANController& can, int8_t steering, int8_t throttle) {
        
		steering = std::clamp(steering, (int8_t)0, (int8_t)120);
		throttle = std::clamp(throttle, (int8_t)-100, (int8_t)100);

		int8_t data[2];

		// Big-endian encoding (network byte order)
		data[0] = steering;	// 0-120 degrees
        data[1] = throttle; // -100 to +100

		can.sendFrameFD(CANID::DRIVE_COMMAND, data, 2);
	}

	inline void sendDriveMode(CANController& can, DriveMode mode) {
        int8_t data[1] = { static_cast<int8_t>(mode) };
        can.sendFrameFD(CANID::DRIVE_MODE, data, 1);
    }
}
