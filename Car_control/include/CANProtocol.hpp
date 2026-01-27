#pragma once

#include "CANController.hpp"

/**
 * @file CANProtocol.hpp
 * @brief CAN IDs and protocol helper functions for sending vehicle commands.
 */

/**
 * @namespace CANID
 * @brief Predefined CAN IDs used by the vehicle communication protocol.
 */
namespace CANID {
	constexpr uint16_t	EMERGENCY_BRAKE = 0x100;	/**< Emergency brake command (max priority) */
	constexpr uint16_t	DRIVE_COMMAND   = 0x101;	/**< Steering and throttle command (high priority) */
	constexpr uint16_t	DRIVE_MODE      = 0x102;	/**< Drive mode selection (low priority) */
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
		
		int16_t data[2] = { steering, throttle };
		can.sendFrame(CANID::DRIVE_COMMAND, data, 2);
	}

	inline void sendDriveMode(CANController& can, DriveMode mode) {

		int16_t data[1] = { static_cast<int16_t>(mode) };
		can.sendFrame(CANID::DRIVE_MODE, data, 1);
	}
}
