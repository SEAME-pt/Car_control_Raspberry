#include "../../include/carControl.h"

std::unique_ptr<CANController>	init_can(const std::string &interface) {

	try {

		std::cout << "Initializing CAN interface: " << interface << std::endl;
		auto can = std::make_unique<CANController>(interface);
		std::cout << "CAN FD initialized successfully!" << std::endl;
		return (can);
	} catch (const CANController::CANException& e) {

		std::cerr << " CAN initialization failed: " << e.what() << std::endl;
		std::cerr << " Hint: Check if interface exists:" << std::endl;
		std::cerr << "   ip link show " << interface << std::endl;
		std::cerr << " For vcan (testing):" << std::endl;
		std::cerr << "   sudo ip link add dev vcan0 type vcan" << std::endl;
		std::cerr << "   sudo ip link set up vcan0" << std::endl;
		return (nullptr);
	} catch (const std::exception& e) {

		// If this message appears, something has gone seriously wrong
		// try delete the folder and code it again, good luck debugging :3
		std::cout << "Unexpected error, weird..." << e.what() << std::endl;
		return (nullptr);
	}
}
