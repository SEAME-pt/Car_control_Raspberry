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
      batteryVoltage(48),
      batteryRange(300),
      motorActive(false),
      motorPower(0) {

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
    out << QString("batteryVoltage") << (qint32)batteryVoltage;
    out << QString("batteryRange") << (qint32)batteryRange;
    out << QString("motorActive") << motorActive;
    out << QString("motorPower") << (qint32)motorPower;

    // Send to all connected clients
    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

void CarDataSender::updateData(int speed, int speedLimit, int batteryLevel, int batteryVoltage, int batteryRange, bool motorActive, int motorPower) {
    this->speed = speed;
    this->speedLimit = speedLimit;
    this->batteryLevel = batteryLevel;
    this->batteryVoltage = batteryVoltage;
    this->batteryRange = batteryRange;
    this->motorActive = motorActive;
    this->motorPower = motorPower;
}