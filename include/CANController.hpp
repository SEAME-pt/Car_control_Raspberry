#pragma once

#include "socketCAN.h"
#include "exceptions.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <stdexcept>

class CANController {

public:
	CANController(int, const std::string &, bool);
	~CANController();

	CANController(const CANController&) = delete;
    CANController& operator=(const CANController&) = delete;

	CANController(CANController&& other) noexcept;
	CANController& operator=(CANController&& other) noexcept;

	void	initialize();
	void	cleanup();

	void	sendFrame(uint32_t can_id, const uint8_t* data, uint8_t len);
	void	sendFrameFD(uint32_t can_id, const uint8_t* data, size_t len);
	void	sendBCMFD(uint32_t can_id, const std::vector<uint8_t>& data, 
                        std::chrono::microseconds interval);
	void	stopPeriodic(uint32_t can_id);

	bool	getInitialized() const;
	bool	getInterface() const;

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
