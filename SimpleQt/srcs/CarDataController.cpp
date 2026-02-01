#include "../incs/CarDataController.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

CarDataController::CarDataController(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_serverAddress("127.0.0.1")
    , m_serverPort(8888)
    , m_isConnected(false)
    , m_speed(0)
    , m_speedLimit(120)
    , m_batteryLevel(85)
    , m_batteryVoltage(48)
    , m_batteryRange(267)
    , m_motorActive(false)
    , m_motorPower(0)
{
    // Setup socket connections
    connect(m_socket, &QTcpSocket::connected, this, &CarDataController::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &CarDataController::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &CarDataController::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &CarDataController::onError);
    
    // Setup reconnect timer
    m_reconnectTimer->setInterval(5000); // Try reconnecting every 5 seconds
    connect(m_reconnectTimer, &QTimer::timeout, this, &CarDataController::attemptReconnect);
    
    // Auto-connect on startup
    QTimer::singleShot(500, this, &CarDataController::connectToServer);
}

CarDataController::~CarDataController() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void CarDataController::setServerAddress(const QString &address) {
    if (m_serverAddress != address) {
        m_serverAddress = address;
        emit serverAddressChanged();
    }
}

void CarDataController::setServerPort(int port) {
    if (m_serverPort != port) {
        m_serverPort = port;
        emit serverPortChanged();
    }
}

void CarDataController::connectToServer() {
    if (m_socket->state() == QAbstractSocket::ConnectedState ||
        m_socket->state() == QAbstractSocket::ConnectingState) {
        qDebug() << "Already connected or connecting";
        return;
    }
    
    qDebug() << "Connecting to" << m_serverAddress << ":" << m_serverPort;
    m_socket->connectToHost(m_serverAddress, m_serverPort);
}

void CarDataController::disconnectFromServer() {
    m_reconnectTimer->stop();
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void CarDataController::reconnect() {
    disconnectFromServer();
    QTimer::singleShot(1000, this, &CarDataController::connectToServer);
}

void CarDataController::onConnected() {
    qDebug() << "Connected to car data server";
    m_isConnected = true;
    m_reconnectTimer->stop();
    emit connectionChanged();
}

void CarDataController::onDisconnected() {
    qDebug() << "Disconnected from car data server";
    m_isConnected = false;
    emit connectionChanged();
     
    // Start reconnection timer
    m_reconnectTimer->start();
}

void CarDataController::onReadyRead() {
    // Read data using QDataStream
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);
    
    while (m_socket->bytesAvailable() > 0) {
        // Read field identifier and value
        QString field;
        in >> field;
        
        if (field == "speed") {
            qint32 value;
            in >> value;
            updateSpeed(value);
        } else if (field == "speedLimit") {
            qint32 value;
            in >> value;
            updateSpeedLimit(value);
        } else if (field == "batteryLevel") {
            qint32 value;
            in >> value;
            updateBatteryLevel(value);
        } else if (field == "batteryVoltage") {
            qint32 value;
            in >> value;
            updateBatteryVoltage(value);
        } else if (field == "batteryRange") {
            qint32 value;
            in >> value;
            updateBatteryRange(value);
        } else if (field == "motorActive") {
            bool value;
            in >> value;
            updateMotorActive(value);
        } else if (field == "motorPower") {
            qint32 value;
            in >> value;
            updateMotorPower(value);
        }
        
        if (in.status() != QDataStream::Ok) {
            qWarning() << "Error reading data stream";
            break;
        }
    }
}

void CarDataController::parseMessage(const QString &message) {
    // Deprecated - now using QDataStream
    Q_UNUSED(message);
}

void CarDataController::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    QString errorString = m_socket->errorString();
    qDebug() << "Socket error:" << errorString;
    emit errorOccurred(errorString);
    
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

void CarDataController::attemptReconnect() {
    qDebug() << "Attempting to reconnect...";
    connectToServer();
}

void CarDataController::updateSpeed(int value) {
    if (m_speed != value) {
        m_speed = value;
        emit speedChanged();
    }
}

void CarDataController::updateSpeedLimit(int value) {
    if (m_speedLimit != value) {
        m_speedLimit = value;
        emit speedLimitChanged();
    }
}

void CarDataController::updateBatteryLevel(int value) {
    if (m_batteryLevel != value) {
        m_batteryLevel = value;
        emit batteryLevelChanged();
    }
}

void CarDataController::updateBatteryVoltage(int value) {
    if (m_batteryVoltage != value) {
        m_batteryVoltage = value;
        emit batteryVoltageChanged();
    }
}

void CarDataController::updateBatteryRange(int value) {
    if (m_batteryRange != value) {
        m_batteryRange = value;
        emit batteryRangeChanged();
    }
}

void CarDataController::updateMotorActive(bool value) {
    if (m_motorActive != value) {
        m_motorActive = value;
        emit motorActiveChanged();
    }
}

void CarDataController::updateMotorPower(int value) {
    if (m_motorPower != value) {
        m_motorPower = value;
        emit motorPowerChanged();
    }
}
