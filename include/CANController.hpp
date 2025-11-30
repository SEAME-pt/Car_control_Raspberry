#pragma once

#include "socketCAN.h"
#include "exceptions.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <memory>

class CANController {

public:
	CANController(const std::string &);
	~CANController();

	CANController(const CANController&) = delete;
    CANController& operator=(const CANController&) = delete;

	CANController(CANController&& other) noexcept;
	CANController& operator=(CANController&& other) noexcept;

	void	initialize();
	void	cleanup();

	//Classical CAN
	void	sendFrame(uint32_t can_id, const int8_t* data, uint8_t len);

	//CAN_FD
	void	sendFrameFD(uint32_t can_id, const int8_t* data, size_t len);

	// Getters
    bool 				isInitialized() const { return _initialized; }
    const std::string&	getInterface() const { return _interface; }
    int 				getSocket() const { return _socket; }

	class CANException : public std::runtime_error {
	public:
    	explicit CANException(const std::string& msg) 
        	: std::runtime_error("CAN Error: " + msg) {}
	};
private:
	int			_socket;
	std::string	_interface;
	bool		_initialized;
};
