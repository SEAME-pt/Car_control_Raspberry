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
	constexpr uint16_t	EMERGENCY_BRAKE 	= 0x100;	//max priority
	constexpr uint16_t	COMMAND_THROTTLE	= 0x101;	//high priority
	constexpr uint16_t	COMMAND_STEERING	= 0x102;	//high priority
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

	inline void sendThrottleCommand(CANController& can, int16_t throttle) {

		can.sendFrame(CANID::COMMAND_THROTTLE, &throttle, sizeof(int16_t));
	}

	inline void sendSteeringCommand(CANController& can, int16_t steering) {
		
		can.sendFrame(CANID::COMMAND_STEERING, &steering, sizeof(int16_t));
	}
}
