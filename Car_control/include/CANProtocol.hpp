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
	constexpr uint16_t	COMMAND_THROTTLE	= 0x101;	/**< Throttle command (high priority) */
	constexpr uint16_t	COMMAND_STEERING	= 0x102;	/**< Steering command (high priority) */
	constexpr uint16_t	HEARTBEAT			= 0x103;	/**< Heartbeat signal (keep-alive) */
};

namespace CANRECEIVERID {
	constexpr uint16_t	SPEEDRPMSTM32			= 0x200;
	constexpr uint16_t	BATTERYSTM32			= 0x201;
}

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
	inline void sendThrottleCommand(CANController& can, int16_t throttle) {

		int8_t data[2];
		data[0] = static_cast<int8_t>(throttle & 0xFF);         // Low byte
		data[1] = static_cast<int8_t>((throttle >> 8) & 0xFF);  // High byte
		can.sendFrame(CANSENDID::COMMAND_THROTTLE, data, 2);
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
		can.sendFrame(CANSENDID::COMMAND_STEERING, data, 2);
	}

	inline uint16_t	receiveSpeedValues(CANController& can) {

		can_frame	rx;
		memset(&rx, 0, sizeof(can_frame));
		
		if (can.receiveFrame(&rx) == 0) {
			if (rx.can_dlc >= 2 && rx.can_id == CANRECEIVERID::SPEEDRPMSTM32) {
				uint16_t rpm = (rx.data[0] << 8) | rx.data[1];
				return (rpmToSpeedMps(rpm));
			} else
				return (-1);
		}
		return (0);
	}

	/**
	 * @brief Sends a heartbeat signal over CAN.
	 *
	 * @param can Reference to an initialized CANController
	 */
	inline void sendHeartbeat(CANController& can) {

    	int8_t data = 0xAA;  // Heartbeat signature
    	can.sendFrame(CANSENDID::HEARTBEAT, &data, 1);
	}
}
