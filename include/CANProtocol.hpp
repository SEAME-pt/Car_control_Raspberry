#pragma once

#include "CANController.hpp"
#include <algorithm>

namespace CANID {
	constexpr uint16_t	EMERGENCY_BRAKE = 0x100;	//max priority
	constexpr uint16_t	DRIVE_COMMAND   = 0x101;	//high priority
	constexpr uint16_t	DRIVE_MODE      = 0x102;	//low priority
};

enum class DriveMode : int16_t {
	MANUAL 		= 0x00,
	AUTONOMOUS	= 0x01
};

namespace CANProtocol {

	inline void sendEmergencyBrake(CANController& can, bool active) {
		int16_t data = active ? 0xFF : 0x00;
		can.sendFrame(CANID::EMERGENCY_BRAKE, &data, 1);
	}

	inline void sendDriveCommand(CANController& can, int16_t steering, int16_t throttle) {
		
		steering = std::clamp(steering, (int16_t)0, (int16_t)120);
		throttle = std::clamp(throttle, (int16_t)-100, (int16_t)100);

		if (steering > -2 && steering < 2)
			steering = 0;
		else if (steering > 57 && steering < 63)
			steering = 60;
		if (throttle > -2 && throttle < 2)
			throttle = 0;

		int16_t data[2];

		// Big-endian encoding (network byte order)
		data[0] = steering;	// 0-120 degrees
		data[1] = throttle; // -100 to +100

		std::cout << "Steering: " << steering << " | Throttle: " << throttle << std::endl;
		can.sendFrame(CANID::DRIVE_COMMAND, data, 2);
	}

	inline void sendDriveMode(CANController& can, DriveMode mode) {
		int16_t data[1] = { static_cast<int16_t>(mode) };
		can.sendFrame(CANID::DRIVE_MODE, data, 1);
	}
}
