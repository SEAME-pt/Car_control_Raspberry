#include "../../include/carControl.h"

std::unique_ptr<CANController>	g_can;

bool	init_can(const std::string &interface) {

	try {		
		g_can = std::make_unique<CANController>(interface);
		std::cout << "CAN FD initialized successfully!" << std::endl;
		return (true);
	} catch (CANController::CANException& e) {
		std::cerr << "CAN initialization failed: " << e.what() << std::endl;
		std::cerr << "Check if interface exists and is UP" << std::endl;
		std::cerr << "sudo ip link set " << interface << " up type can bitrate 500000 fd on" << std::endl;
	
		g_can.reset();
		return (false);
	}
}

CANController*	get_can() {
	return g_can.get();
}
