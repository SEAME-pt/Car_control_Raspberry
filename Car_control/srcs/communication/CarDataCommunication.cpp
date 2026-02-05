#include "CarDataCommunication.hpp"

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QDebug>

CarDataCommunication::CarDataCommunication(quint16 port, QObject *parent)
    : QObject(parent), 
      server(nullptr),
      clientSocket(nullptr),
      timer(new QTimer(this)),
      isServerMode(port > 0),
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

    if (isServerMode) {
        // Server mode: listen for incoming connections
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &CarDataCommunication::onNewConnection);

        if (server->listen(QHostAddress::Any, port)) {
            qDebug() << "CarDataCommunication Server listening on port" << port;
        } else {
            qDebug() << "Failed to start server:" << server->errorString();
        }
    } else {
        // Client mode: socket will be created when connectToHost() is called
        qDebug() << "CarDataCommunication initialized in client mode";
    }

    // Start timer to send data periodically
    connect(timer, &QTimer::timeout, this, &CarDataCommunication::sendData);
    timer->start(500); // Send every 500ms
}

void CarDataCommunication::connectToHost(const QString &host, quint16 port) {
    if (isServerMode) {
        qWarning() << "Cannot connectToHost in server mode";
        return;
    }

    if (!clientSocket) {
        clientSocket = new QTcpSocket(this);
        connect(clientSocket, &QTcpSocket::connected, this, &CarDataCommunication::onConnected);
        connect(clientSocket, &QTcpSocket::disconnected, this, &CarDataCommunication::onDisconnected);
        connect(clientSocket, &QTcpSocket::readyRead, this, &CarDataCommunication::onReadyRead);
        connect(clientSocket, &QTcpSocket::errorOccurred, this, &CarDataCommunication::onSocketError);
    }

    qDebug() << "Connecting to" << host << ":" << port;
    clientSocket->connectToHost(host, port);
}

void CarDataCommunication::disconnectFromHost() {
    if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
        clientSocket->disconnectFromHost();
    }
}

void CarDataCommunication::setCruiseControl(int value) {
    if (speed != value) {
        speed = value;
        emit cruiseControlChanged();  // Q_PROPERTY notification
    }
}

void CarDataCommunication::onNewConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    clients.append(client);
    
    connect(client, &QTcpSocket::disconnected, this, &CarDataCommunication::onClientDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &CarDataCommunication::onReadyRead);
    qDebug() << "New client connected";
    emit connected();
}

void CarDataCommunication::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        clients.removeAll(client);
        client->deleteLater();
        qDebug() << "Client disconnected";
        emit disconnected();
    }
}

void CarDataCommunication::onConnected() {
    qDebug() << "Connected to server";
    emit connected();
}

void CarDataCommunication::onDisconnected() {
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void CarDataCommunication::onSocketError() {
    if (clientSocket) {
        qWarning() << "Socket error:" << clientSocket->errorString();
        emit errorOccurred(clientSocket->errorString());
    }
}

void CarDataCommunication::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // Read incoming data using QDataStream
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    
    while (socket->bytesAvailable() > 0) {
        QString field;
        in >> field;
        
        if (field == "cruiseControl") {
            qint32 value;
            in >> value;
            setCruiseControl(value);
            emit fieldReceived("cruiseControl", value);
        }
        
        if (in.status() != QDataStream::Ok) {
            qWarning() << "Error reading data stream";
            break;
        }
    }
}

void CarDataCommunication::sendData() {
    // Skip if no connections (server mode with no clients, or client mode disconnected)
    if (isServerMode && clients.isEmpty()) {
        return;
    }
    if (!isServerMode && (!clientSocket || clientSocket->state() != QAbstractSocket::ConnectedState)) {
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

    // Send to all connected clients (server mode) or to server (client mode)
    if (isServerMode) {
        for (QTcpSocket *client : clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
                client->flush();
            }
        }
    } else if (clientSocket) {
        clientSocket->write(data);
        clientSocket->flush();
    }
}

void CarDataCommunication::updateData(int speed, int speedLimit, int batteryLevel, int batteryRange, 
                              bool motorActive, int motorPower, double temperature, 
                              double totalDistance, bool showError, 
                              const QString &errorMessage) {
    setSpeed(speed);  // Use setter to trigger Q_PROPERTY notification
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

void CarDataCommunication::updateSpeed(int value) {
    setSpeed(value);  // Use setter to trigger Q_PROPERTY notification
}

void CarDataCommunication::updateSpeedLimit(int value) {
    this->speedLimit = value;
}

void CarDataCommunication::updateBatteryLevel(int value) {
    this->batteryLevel = value;
}

void CarDataCommunication::updateBatteryRange(int value) {
    this->batteryRange = value;
}

void CarDataCommunication::updateMotorActive(bool value) {
    this->motorActive = value;
}

void CarDataCommunication::updateMotorPower(int value) {
    this->motorPower = value;
}

void CarDataCommunication::updateTemperature(double value) {
    this->temperature = value;
}

void CarDataCommunication::updateTotalDistance(double value) {
    this->totalDistance = value;
}

void CarDataCommunication::updateShowError(bool value) {
    this->showError = value;
}

void CarDataCommunication::updateErrorMessage(const QString &value) {
    this->errorMessage = value;
}
