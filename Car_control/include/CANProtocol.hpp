#pragma once

#include "CANController.hpp"

uint16_t rpmToSpeedMps(uint16_t rpm);

/**
 * @file CANProtocol.hpp
 * @brief CAN IDs and protocol helper functions for sending vehicle commands.
 */

/**
 * @namespace CANSENDID
 * @brief Predefined CAN IDs used by the vehicle communication protocol to send messages.
 */
namespace CANSENDID {
	constexpr uint16_t	EMERGENCY_BRAKE 	= 0x100;	/**< Emergency brake command (max priority) */
	constexpr uint16_t	DRIVING_COMMAND		= 0x101;	/**< driving command (medium prority) */
};

namespace CANRECEIVERID {
	constexpr uint16_t	SPEEDRPMSTM32			= 0x200;
	constexpr uint16_t	BATTERYSTM32			= 0x201;
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
		can.sendFrame(CANSENDID::EMERGENCY_BRAKE, &data, 1);
	}

	/**
	 * @brief Sends a throttle command over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 * @param throttle Throttle value to send
	 */
	inline void sendDrivingCommand(CANController& can, int16_t throttle, int16_t steering) {

		int8_t data[2];
		data[0] = static_cast<int8_t>(throttle & 0xFF);         // Low byte
		data[1] = static_cast<int8_t>((throttle >> 8) & 0xFF);  // High byte
		data[2] = static_cast<int8_t>(steering & 0xFF);         // Steering Low byte
    	data[3] = static_cast<int8_t>((steering >> 8) & 0xFF);  // Steering High byte
		can.sendFrame(CANSENDID::DRIVING_COMMAND, data, 4);
	}
}
