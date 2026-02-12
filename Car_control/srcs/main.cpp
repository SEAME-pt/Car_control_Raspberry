#include "carControl.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include "communication/CarDataCommunication.hpp"

std::unique_ptr<CarDataCommunication> comm;

void	qDataStreamThread(t_carControl* carControl, t_CANReceiver* canReceiver, int argc, char *argv[])
{
	t_speedData	speedData;
	t_batteryData	batteryData;
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
	batteryData.percentage = 0;
	batteryData.voltage = 0;
	speedData.rpm = 0;
	speedData.speedMps = 0;
	while (g_running.load()) {
		// Process Qt events (handles connections, signals, etc)
		carControl->app->processEvents();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		carControl->comm->updateSpeed(getSpeedData(canReceiver, &speedData, false) ? speedData.rpm : 0);
		carControl->comm->updateBatteryLevel(getBatteryData(canReceiver, &batteryData, false) ? batteryData.percentage : 80);
		// Immediately send updated data
		carControl->comm->sendData();
		qDebug() << "Updated data # " << batteryData.percentage << "%, Voltage: " << batteryData.voltage << "V";
		qDebug() << "Updated data # " << speedData.speedMps << " m/s, RPM: " << speedData.rpm;

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

	// Initialize CAN receiver
    t_CANReceiver canReceiver;
    canReceiver.can = carControl.can.get();

	// Launch Qt data stream thread (pass argc/argv to allow creating QCoreApplication inside thread)
	std::thread qtThread(qDataStreamThread, &carControl, &canReceiver, argc, argv);



	// Threads launcher
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
