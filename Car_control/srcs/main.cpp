#include "carControl.h"

int	main(int argc, char *argv[]) {
	signalManager();
	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);		

	// Initialize CAN receiver
    t_CANReceiver canReceiver;
    canReceiver.can = carControl.can.get();
	
	// Threads launcher
	std::thread qtThread(qDataStreamThread, &carControl, &canReceiver, argc, argv);
    std::thread rxThread(canReceiverThread, &canReceiver);
    std::thread monitorThread(monitoringThread, &canReceiver);

	if (qtThread.joinable())
		qtThread.join();

    if (rxThread.joinable())
        rxThread.join();

    if (monitorThread.joinable())
        monitorThread.join();
	
	try {
		if (!carControl.manual) {
			std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
			manualLoop(&carControl);
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}


	try {
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
	} catch (...) {
		return (1);
	}
	std::cout << "Emergency brake sent, exiting..." << std::endl;
}
