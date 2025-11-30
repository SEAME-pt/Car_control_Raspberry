#include "../../include/CANController.hpp"

CANController::CANController(int s, const std::string &interface,
		bool inicialized) 
		: _socket(-1) 
		, _interface(interface) 
		, _initialized(false) {

	initialize();
}

CANController::~CANController() {
	cleanup();
}

CANController::CANController(CANController&& other) noexcept
	: _socket(other._socket)
	, _interface(std::move(other._interface))
	, _initialized(other._initialized) {

	other._socket = -1;
	other._initialized = false;
}

CANController& CANController::operator=(CANController&& other) noexcept {

	if (this != &other) {
		cleanup();
		_socket = other._socket;
		_interface = std::move(other._interface);
		_initialized = other._initialized;
		
		other._socket = -1;
		other._initialized = false;
	}
	return (*this);
}

void	CANController::initialize() {

	if (_initialized) {
		std::cerr << "CAN already initialized" << std::endl;
		return ;
	}

	_socket = socketCan_init(_interface.c_str());
	if (_socket < 0) {
		throw CANController::CANException("Failed to inicialize interface: "
		+ _interface);
	}
	_initialized = true;
	std::cout << "SocketCAN initialized" << std::endl;
}

void	CANController::cleanup() {
	
	if (_initialized && _socket >= 0) {
		can_close(_socket);
		_socket = -1;
		_initialized = false;
		std::cout << "CANController cleaned up" << std::endl;
	}
}

void	CANController::sendFrame(uint32_t can_id, 
			const int8_t* data, uint8_t len) {

	if (!_initialized)
		throw CANException("CAN not initialized");
	
	if (can_send_frame(_socket, can_id, data, len) < 0) {
		throw CANException("Failed to send frame (ID: 0x" +
		std::to_string(can_id) + ")");
	}
}

void	CANController::sendFrameFD(uint32_t can_id, 
			const int8_t* data, size_t len) {

	if (!_initialized)
		throw CANException("CAN not initialized");

	if (can_send_frame_fd(_socket, can_id, data, len) < 0) {
		throw CANException("Failed to send frame (ID: 0x" +
		std::to_string(can_id) + ")");
	}
}
