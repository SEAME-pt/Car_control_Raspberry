#include "CANController.hpp"

// Constructor
CANController::CANController(const std::string &interface) 
	: _interface(interface) {

	_socket = -1;
	_initialized = false;
	initialize();
}

// Destructor
CANController::~CANController() {
	cleanup();
}

// Move Constructor
CANController::CANController(CANController&& other) noexcept
	: _socket(other._socket)
	, _interface(std::move(other._interface))
	, _initialized(other._initialized) {

	other._socket = -1;
	other._initialized = false;
}

// Move Assignment Operator
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

// Abstraction layer for CAN socket initialization
void	CANController::initialize() {

	if (_initialized) {
		std::cerr << "CAN already initialized, nothing to do here..." 
		<< std::endl;
		return ;
	}

	_socket = socketCan_init(_interface.c_str());
	if (_socket < 0) {
		throw CANException("Failed to initialize interface: "
		+ _interface);
	}
	_initialized = true;
}

void	CANController::cleanup() {
	
	if (_initialized && _socket >= 0) {
		can_close(_socket);
		_socket = -1;
		_initialized = false;
	}
}

// TX handler sending frames in classic CAN format
void	CANController::sendFrame(uint16_t can_id, 
			const int16_t* data, uint8_t len) {

	if (!_initialized)
		throw CANException("CAN not initialized");
	
	if (can_send_frame(_socket, can_id, data, len) < 0) {
		throw CANException("Failed to send frame (ID: 0x" +
		std::to_string(can_id) + ")");
	}
}

// TX handler sending frames in CAN_FD format
void	CANController::sendFrameFD(uint16_t can_id, 
			const int16_t* data, uint8_t len) {

	if (!_initialized)
		throw CANException("CAN not initialized");

	if (can_send_frame_fd(_socket, can_id, data, len) < 0) {
		throw CANException("Failed to send frame (ID: 0x" +
		std::to_string(can_id) + ")");
	}
}

// Reads incoming messages and stores all information on struct
// -1 returned on error
int		CANController::receiveFrame(struct can_frame *frame) {

	return (can_try_receive(_socket, frame));
}

int		CANController::receiveFrameFD(struct canfd_frame *frame) {

	return (canfd_try_receive(_socket, frame));
}
