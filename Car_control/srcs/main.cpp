#include "carControl.h"

int	main(int argc, char *argv[]) {

	signalManager();

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);

	// Initialize CAN receiver
    t_CANReceiver canReceiver;
    canReceiver.can = carControl.can.get();

	// Launch CAN receiver thread (reads all incoming messages)
    std::thread rxThread(canReceiverThread, &canReceiver);

	// Launch monitoring thread (sends heartbeat via EMERGENCY_BRAKE(false), monitors STM32 health)
    std::thread monitorThread(monitoringThread, &canReceiver);

	try {
		if (!carControl.manual) {
			std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
			manualLoop(&carControl, &canReceiver);
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	// Stop all threads
    g_running.store(false);
    
    if (rxThread.joinable()) {
        rxThread.join();
    }
    if (monitorThread.joinable()) {
        monitorThread.join();
    }

	try {
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
	} catch (...) {
		return (1);
	}
	return (0);
}
