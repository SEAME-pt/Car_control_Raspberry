#include "CarDataSender.hpp"

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

CarDataSender::CarDataSender(quint16 port, QObject *parent)
    : QObject(parent), 
      server(new QTcpServer(this)),
      timer(new QTimer(this)),
      speed(0),
      speedLimit(120),
      batteryLevel(100),
      batteryRange(300),
      motorActive(false),
      motorPower(0),
      temperature(20.0),
      totalDistance(0.0),
      showError(false),
      errorMessage("") {

    connect(server, &QTcpServer::newConnection, this, &CarDataSender::onNewConnection);

    if (server->listen(QHostAddress::Any, port)) {
        qDebug() << "Car Data Server listening on port" << port;
    } else {
        qDebug() << "Failed to start server:" << server->errorString();
    }

    // Start timer to send data periodically
    connect(timer, &QTimer::timeout, this, &CarDataSender::sendData);
    timer->start(500); // Send every 500ms
}

void CarDataSender::onNewConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    clients.append(client);
    
    connect(client, &QTcpSocket::disconnected, this, &CarDataSender::onClientDisconnected);
}

void CarDataSender::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        clients.removeAll(client);
        client->deleteLater();
    }
}

void CarDataSender::sendData() {
    if (clients.isEmpty()) {
        return;
    }

    // Prepare data using QDataStream
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << QString("speed") << (qint32)speed;
    out << QString("speedLimit") << (qint32)speedLimit;
    out << QString("batteryLevel") << (qint32)batteryLevel;
    out << QString("batteryRange") << (qint32)batteryRange;
    out << QString("motorActive") << motorActive;
    out << QString("motorPower") << (qint32)motorPower;
    out << QString("temperature") << temperature;
    out << QString("totalDistance") << totalDistance;
    out << QString("showError") << showError;
    out << QString("errorMessage") << errorMessage;

    // Send to all connected clients
    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

void CarDataSender::updateData(int speed, int speedLimit, int batteryLevel, int batteryRange, 
                              bool motorActive, int motorPower, double temperature, 
                              double totalDistance, bool showError, 
                              const QString &errorMessage) {
    this->speed = speed;
    this->speedLimit = speedLimit;
    this->batteryLevel = batteryLevel;
    this->batteryRange = batteryRange;
    this->motorActive = motorActive;
    this->motorPower = motorPower;
    this->temperature = temperature;
    this->totalDistance = totalDistance;
    this->showError = showError;
    this->errorMessage = errorMessage;
}

void CarDataSender::updateSpeed(int value) {
    this->speed = value;
}

void CarDataSender::updateSpeedLimit(int value) {
    this->speedLimit = value;
}

void CarDataSender::updateBatteryLevel(int value) {
    this->batteryLevel = value;
}

void CarDataSender::updateBatteryRange(int value) {
    this->batteryRange = value;
}

void CarDataSender::updateMotorActive(bool value) {
    this->motorActive = value;
}

void CarDataSender::updateMotorPower(int value) {
    this->motorPower = value;
}

void CarDataSender::updateTemperature(double value) {
    this->temperature = value;
}

void CarDataSender::updateTotalDistance(double value) {
    this->totalDistance = value;
}

void CarDataSender::updateShowError(bool value) {
    this->showError = value;
}

void CarDataSender::updateErrorMessage(const QString &value) {
    this->errorMessage = value;
}