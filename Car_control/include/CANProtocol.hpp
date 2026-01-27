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
	constexpr uint16_t	EMERGENCY_BRAKE 	= 0x100;	/**< Emergency brake command (max priority) */
	constexpr uint16_t	COMMAND_THROTTLE	= 0x101;	/**< Throttle command (high priority) */
	constexpr uint16_t	COMMAND_STEERING	= 0x102;	/**< Steering command (high priority) */
};

/**
 * @namespace CANProtocol
 * @brief High-level helper functions to send CAN messages.
 */
namespace CANProtocol {

	/**
	 * @brief Sends an emergency brake command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param active True to activate brake, false to release
	 */
	inline void sendEmergencyBrake(CANController& can, bool active) {

		int16_t data = active ? 0xFF : 0x00;
		can.sendFrame(CANID::EMERGENCY_BRAKE, &data, 1);
	}

	/**
	 * @brief Sends a throttle command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param throttle Throttle value to send
	 */
	inline void sendThrottleCommand(CANController& can, int16_t throttle) {

		can.sendFrame(CANID::COMMAND_THROTTLE, &throttle, sizeof(int16_t));
	}

	/**
	 * @brief Sends a steering command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param steering Steering value to send
	 */
	inline void sendSteeringCommand(CANController& can, int16_t steering) {
		
		can.sendFrame(CANID::COMMAND_STEERING, &steering, sizeof(int16_t));
	}
}
