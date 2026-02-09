#include "carControl.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include "communication/CarDataCommunication.hpp"

std::unique_ptr<CarDataCommunication> comm;

void	qDataStreamThread(t_carControl* carControl, int argc, char *argv[])
{
	t_speedData	speedData;
	t_batteryData	batteryData;
	t_CANReceiver	canReceiver;
	canReceiver.can = carControl->can.get();
	// Create QCoreApplication in this thread so it owns the event loop
	try {
		carControl->app = std::make_unique<QCoreApplication>(argc, argv);
	} catch (const std::exception &e) {
		std::cerr << "Failed to create QCoreApplication in thread: " << e.what() << std::endl;
		return;
	}

	// Create CarDataCommunication in this thread so sockets belong here
	try {
		carControl->comm = std::make_unique<CarDataCommunication>(8888);
	} catch (const std::exception &e) {
		std::cerr << "Failed to create CarDataCommunication in thread: " << e.what() << std::endl;
		return;
	}

	// Connect signals now that comm exists
	QObject::connect(carControl->comm.get(), &CarDataCommunication::fieldReceived,
		[](const QString &fieldName, const QVariant &value) {
			qDebug() << "Received field:" << fieldName << "=" << value;
		});

	QObject::connect(carControl->comm.get(), &CarDataCommunication::connected,
		[]() { qDebug() << "Client connected to server"; });

	QObject::connect(carControl->comm.get(), &CarDataCommunication::disconnected,
		[]() { qDebug() << "Client disconnected from server"; });

	int counter = 0;
	while (g_running.load()) {
		// Process Qt events (handles connections, signals, etc)
		carControl->app->processEvents();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		carControl->comm->updateSpeed(getSpeedData(&canReceiver, &speedData) ? speedData.rpm : 0);
		carControl->comm->updateBatteryLevel(getBatteryData(&canReceiver, &batteryData) ? batteryData.percentage : 100);
		// Immediately send updated data
		carControl->comm->sendData();
		qDebug() << "Updated data #" << counter;
		counter++;
	}

	// Clean up in this thread
	carControl->comm.reset();
	carControl->app.reset();
}


int	main(int argc, char *argv[]) {
	signalManager();
	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);		
	
	// Launch Qt data stream thread (pass argc/argv to allow creating QCoreApplication inside thread)
	std::thread qtThread(qDataStreamThread, &carControl, argc, argv);

	qtThread.join();
	/*
	// Initialize CAN receiver
    t_CANReceiver canReceiver;
    canReceiver.can = carControl.can.get();

	// Launch CAN receiver thread (reads all incoming messages)
    std::thread rxThread(canReceiverThread, &canReceiver);

	// Launch monitoring thread (sends heartbeat via EMERGENCY_BRAKE(false), monitors STM32 health)
    std::thread monitorThread(monitoringThread, &canReceiver);

	// Stop all threads
    g_running.store(false);
    
    if (rxThread.joinable()) {
        rxThread.join();
    }
	std::cout << "Emergency brake sent, exiting..." << std::endl;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
	std::cout << "Emergency brake sent, exiting..." << std::endl;

	try {
		CANProtocol::sendEmergencyBrake(*carControl.can, true);
	} catch (...) {
		return (1);
	}
	std::cout << "Emergency brake sent, exiting..." << std::endl;
	return app.exec();
	*/
}
