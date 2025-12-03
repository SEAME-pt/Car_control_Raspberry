#include "../../include/carControl.h"

std::unique_ptr<CANController>	init_can(const std::string &interface) {

	std::cout << "Initializing CAN interface: " << interface << std::endl;
	auto can = std::make_unique<CANController>(interface);
	std::cout << "CAN FD initialized successfully!" << std::endl;
	return (can);
}
