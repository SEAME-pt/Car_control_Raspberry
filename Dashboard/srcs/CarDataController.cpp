#include "../incs/CarDataController.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QSettings>
#include <QtMath>

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
    , m_temperature(20.0)
    , m_totalDistance(0.0)
    , m_showError(false)
    , m_errorMessage("")
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

    // Load persisted settings
    QSettings settings;
    m_distanceUnit = settings.value("settings/distanceUnit", "Km").toString();
    m_temperatureUnit = settings.value("settings/temperatureUnit", "Celsius").toString();
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

void CarDataController::setDistanceUnit(const QString &unit) {
    if (m_distanceUnit != unit) {
        m_distanceUnit = unit;
        QSettings settings;
        settings.setValue("settings/distanceUnit", m_distanceUnit);
        emit distanceUnitChanged();
    }
        // Always notify speed and speed limit changed so UI can refresh display
        emit speedChanged();
        emit speedLimitChanged();
}

void CarDataController::setTemperatureUnit(const QString &unit) {
    if (m_temperatureUnit != unit) {
        m_temperatureUnit = unit;
        QSettings settings;
        settings.setValue("settings/temperatureUnit", m_temperatureUnit);
        emit temperatureUnitChanged();
    }
        // Always notify temperature changed so UI can refresh display
        emit temperatureChanged();
}

int CarDataController::speed() const {
    if (m_distanceUnit.toLower() == "miles") {
        return qRound(m_speed * 0.621371);
    }
    return m_speed;
}

int CarDataController::speedLimit() const {
    if (m_distanceUnit.toLower() == "miles") {
        return qRound(m_speedLimit * 0.621371);
    }
    return m_speedLimit;
}

double CarDataController::totalDistance() const {
    if (m_distanceUnit.toLower() == "miles") {
        return m_totalDistance * 0.621371;
    }
    return m_totalDistance;
}

double CarDataController::temperature() const {
    if (m_temperatureUnit.toLower() == "fahrenheit") {
        return (m_temperature * 9.0 / 5.0) + 32.0;
    }
    return m_temperature;
}

void CarDataController::connectToServer() {
    if (m_socket->state() == QAbstractSocket::ConnectedState ||
        m_socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }
    
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
    m_isConnected = true;
    m_reconnectTimer->stop();
    emit connectionChanged();
}

void CarDataController::onDisconnected() {
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
        } else if (field == "temperature") {
            double value;
            in >> value;
            updateTemperature(value);
        } else if (field == "totalDistance") {
            double value;
            in >> value;
            updateTotalDistance(value);
        } else if (field == "showError") {
            bool value;
            in >> value;
            updateShowError(value);
        } else if (field == "errorMessage") {
            QString value;
            in >> value;
            updateErrorMessage(value);
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
    connectToServer();
}

void CarDataController::updateSpeed(int value) {
    if (m_speed != value) {
        m_speed = value;
        qDebug() << "Speed updated to:" << m_speed;
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

void CarDataController::updateTemperature(double value) {
    if (m_temperature != value) {
        m_temperature = value;
        emit temperatureChanged();
    }
}

void CarDataController::updateTotalDistance(double value) {
    if (m_totalDistance != value) {
        m_totalDistance = value;
        emit totalDistanceChanged();
    }
}

void CarDataController::updateShowError(bool value) {
    if (m_showError != value) {
        m_showError = value;
        emit showErrorChanged();
    }
}

void CarDataController::updateErrorMessage(const QString &value) {
    if (m_errorMessage != value) {
        m_errorMessage = value;
        emit errorMessageChanged();
    }
}

void CarDataController::dismissError() {
    updateShowError(false);
    updateErrorMessage("");
}

void CarDataController::sendSpeedLimit(int value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("speedLimit") << (qint32)value;
    
    m_socket->write(data);
    m_socket->flush();
    qDebug() << "Sent speedLimit:" << value;
}

void CarDataController::sendBatteryLevel(int value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("batteryLevel") << (qint32)value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendBatteryRange(int value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("batteryRange") << (qint32)value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendMotorActive(bool value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("motorActive") << value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendMotorPower(int value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("motorPower") << (qint32)value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendTemperature(double value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("temperature") << value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendTotalDistance(double value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("totalDistance") << value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendShowError(bool value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("showError") << value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendErrorMessage(const QString &value) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("errorMessage") << value;
    
    m_socket->write(data);
    m_socket->flush();
}

void CarDataController::sendAllData() {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send: not connected";
        return;
    }
    
    // Send all current values in one batch
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    
    out << QString("speed") << (qint32)m_speed;
    out << QString("speedLimit") << (qint32)m_speedLimit;
    out << QString("batteryLevel") << (qint32)m_batteryLevel;
    out << QString("batteryRange") << (qint32)m_batteryRange;
    out << QString("motorActive") << m_motorActive;
    out << QString("motorPower") << (qint32)m_motorPower;
    out << QString("temperature") << m_temperature;
    out << QString("totalDistance") << m_totalDistance;
    out << QString("showError") << m_showError;
    out << QString("errorMessage") << m_errorMessage;
    
    m_socket->write(data);
    m_socket->flush();
    qDebug() << "Sent all data to server";
}
