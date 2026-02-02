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

		int8_t data = active ? 0xF : 0x00;
		can.sendFrame(CANID::EMERGENCY_BRAKE, &data, 1);
	}

	/**
	 * @brief Sends a throttle command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param throttle Throttle value to send
	 */
	inline void sendThrottleCommand(CANController& can, int16_t throttle) {

		int8_t data[2];
    	data[0] = static_cast<int8_t>(throttle & 0xFF);         // Low byte
    	data[1] = static_cast<int8_t>((throttle >> 8) & 0xFF);  // High byte
		can.sendFrame(CANID::COMMAND_THROTTLE, data, 2);
	}

	/**
	 * @brief Sends a steering command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param steering Steering value to send
	 */
	inline void sendSteeringCommand(CANController& can, int16_t steering) {
		
		int8_t data[2];
    	data[0] = static_cast<int8_t>(steering & 0xFF);         // Low byte
    	data[1] = static_cast<int8_t>((steering >> 8) & 0xFF);  // High byte
		can.sendFrame(CANID::COMMAND_STEERING, data, 2);
	}
}
