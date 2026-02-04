#include "carControl.h"
#include <QCoreApplication>
#include <QTimer>
#include "communication/CarDataSender.hpp"

CarDataSender *sender;
int	main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);

	// Create the data sender (server listening on port 8888)
	sender = new CarDataSender(8888);

	// Timer to update and send data periodically
	QTimer *updateTimer = new QTimer();
	QObject::connect(updateTimer, &QTimer::timeout, [&]() {
		// TODO: Update with real data from carControl
		sender->updateData(50, 120, 80, 48, 240, true, 60);
	});
	updateTimer->start(500); // Send every 500ms

	/*signalManager();

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);

	try {
		if (!carControl.manual) {
			std::cout << "Autonomous mode chosed, initiating ai..." << std::endl;
			autonomousLoop(carControl);
		} else {
			std::cout << "Manual mode chosed, initiating joystick..." << std::endl;
			manualLoop(&carControl);
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	CANProtocol::sendEmergencyBrake(*carControl.can, true);*/
	
	return app.exec();
}
