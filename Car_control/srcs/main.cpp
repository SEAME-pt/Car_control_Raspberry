#include "carControl.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include "communication/CarDataCommunication.hpp"

CarDataCommunication *comm;

int	main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);

	// Create the data communication (server listening on port 8888)
	comm = new CarDataCommunication(8888);

	// Connect to receive incoming data from clients
	QObject::connect(comm, &CarDataCommunication::fieldReceived, 
		[](const QString &fieldName, const QVariant &value) {
			qDebug() << "Received field:" << fieldName << "=" << value;
			
			// Example: Handle specific fields
			if (fieldName == "speed") {
				qDebug() << "Speed updated to:" << value.toInt();
			}
			// Add more field handlers as needed
		});

	// Connect to monitor connection status
	QObject::connect(comm, &CarDataCommunication::connected, 
		[]() {
			qDebug() << "Client connected to server";
		});
	
	QObject::connect(comm, &CarDataCommunication::disconnected, 
		[]() {
			qDebug() << "Client disconnected from server";
		});

	// Timer to update and send data periodically
	QTimer *updateTimer = new QTimer();
	QObject::connect(updateTimer, &QTimer::timeout, [&]() {
		// TODO: Update with real data from carControl
		comm->updateData(50, 120, 80, 240, true, 60, 22.5, 1234.5, false, "");
		
		// Example: Use Q_PROPERTY setter which emits speedChanged signal
		comm->setSpeed(55);  // This will notify any QML bindings or other listeners
	});
	updateTimer->start(500); // Send every 500ms

	/*signalManager();
W
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
	}*/
	return app.exec();
}
