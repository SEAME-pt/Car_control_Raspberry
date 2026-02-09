#pragma once

#include "socketCAN.h"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <mutex>

/**
 * @file CANController.hpp
 * @brief High-level C++ wrapper for SocketCAN communication.
 *
 * Provides initialization, cleanup, sending and receiving
 * of classical CAN and CAN-FD frames, with RAII support.
 */
class CANController {

public:
	/**
	 * @brief Constructor
	 *
	 * Initializes the CAN interface.
	 *
	 * @param interface Name of the CAN interface (e.g., "can0")
	 * @throws CANException if initialization fails
	 */
	explicit CANController(const std::string &interface);

	/**
	 * @brief Destructor
	 *
	 * Cleans up and closes the CAN socket if initialized.
	 */
	~CANController();

	// Delete copy semantics
	CANController(const CANController&) = delete;
	CANController& operator=(const CANController&) = delete;

	/**
	 * @brief Move constructor
	 *
	 * Transfers ownership of the socket and state.
	 */
	CANController(CANController&& other) noexcept;

	/**
	 * @brief Move assignment operator
	 *
	 * Transfers ownership of the socket and state.
	 */
	CANController& operator=(CANController&& other) noexcept;

	/**
	 * @brief Initializes the CAN interface
	 *
	 * Creates and binds the socket using SocketCAN.
	 * Safe to call multiple times; subsequent calls are ignored.
	 *
	 * @throws CANException on failure
	 */
	void	initialize();

	/**
	 * @brief Cleans up the CAN interface
	 *
	 * Closes the socket and resets internal state.
	 */
	void	cleanup();

	/**
	 * @brief Sends a classical CAN frame (up to 8 bytes)
	 *
	 * @param can_id 11-bit CAN identifier
	 * @param data Pointer to the data buffer
	 * @param len Number of bytes to send (max 8)
	 * @throws CANException if CAN is not initialized or send fails
	 */
	void	sendFrame(uint16_t can_id, const int8_t* data, uint8_t len);

	/**
	 * @brief Sends a CAN-FD frame (up to 64 bytes)
	 *
	 * @param can_id 11-bit CAN identifier
	 * @param data Pointer to the data buffer
	 * @param len Number of bytes to send (max 64)
	 * @throws CANException if CAN is not initialized or send fails
	 */
	void	sendFrameFD(uint16_t can_id, const int16_t* data, uint8_t len);

	/**
	 * @brief Attempts to receive a classical CAN frame (non-blocking)
	 *
	 * @param frame Pointer to struct can_frame to store received data
	 * @return 0 if a frame was read, -1 if no data
	 */
	int		receiveFrame(struct can_frame *frame);

	/**
	 * @brief Attempts to receive a CAN-FD frame (non-blocking)
	 *
	 * @param frame Pointer to struct canfd_frame to store received data
	 * @return 0 if a frame was read, -1 if no data
	 */
	int		receiveFrameFD(struct canfd_frame *frame);

	// Getters
	bool 				isInitialized() const { return _initialized; }	/**< Returns true if CAN is initialized */
	const std::string&	getInterface() const { return _interface; }		/**< Returns interface name */
	int 				getSocket() const { return _socket; }			/**< Returns socket file descriptor */

	/**
	 * @class CANException
	 * @brief Custom exception for CAN errors
	 */
	class CANException : public std::runtime_error {
	public:
		explicit CANException(const std::string& msg) 
			: std::runtime_error("CAN Error: " + msg) {}
	};
private:
	int					_socket;		/**< CAN socket file descriptor */
	std::string			_interface;		/**< CAN interface name */
	bool				_initialized;	/**< Indicates if CAN is initialized */
	mutable std::mutex	_mutex;			/**< Protects CAN socket access */
};
