#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QString>
#include <QDataStream>

class CarDataController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int speedLimit READ speedLimit NOTIFY speedLimitChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(int batteryVoltage READ batteryVoltage NOTIFY batteryVoltageChanged)
    Q_PROPERTY(int batteryRange READ batteryRange NOTIFY batteryRangeChanged)
    Q_PROPERTY(bool motorActive READ motorActive NOTIFY motorActiveChanged)
    Q_PROPERTY(int motorPower READ motorPower NOTIFY motorPowerChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)

public:
    explicit CarDataController(QObject *parent = nullptr);
    ~CarDataController();
    
    int speed() const { return m_speed; }
    int speedLimit() const { return m_speedLimit; }
    int batteryLevel() const { return m_batteryLevel; }
    int batteryVoltage() const { return m_batteryVoltage; }
    int batteryRange() const { return m_batteryRange; }
    bool motorActive() const { return m_motorActive; }
    int motorPower() const { return m_motorPower; }
    bool isConnected() const { return m_isConnected; }
    QString serverAddress() const { return m_serverAddress; }
    int serverPort() const { return m_serverPort; }
    
    void setServerAddress(const QString &address);
    void setServerPort(int port);

public slots:
    void connectToServer();
    void disconnectFromServer();
    void reconnect();

signals:
    void speedChanged();
    void speedLimitChanged();
    void batteryLevelChanged();
    void batteryVoltageChanged();
    void batteryRangeChanged();
    void motorActiveChanged();
    void motorPowerChanged();
    void connectionChanged();
    void serverAddressChanged();
    void serverPortChanged();
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void attemptReconnect();

private:
    void parseMessage(const QString &message);
    void updateSpeed(int value);
    void updateSpeedLimit(int value);
    void updateBatteryLevel(int value);
    void updateBatteryVoltage(int value);
    void updateBatteryRange(int value);
    void updateMotorActive(bool value);
    void updateMotorPower(int value);
    
    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;
    
    QString m_serverAddress;
    int m_serverPort;
    bool m_isConnected;
    
    int m_speed;
    int m_speedLimit;
    int m_batteryLevel;
    int m_batteryVoltage;
    int m_batteryRange;
    bool m_motorActive;
    int m_motorPower;
    
    QString m_buffer;
};
