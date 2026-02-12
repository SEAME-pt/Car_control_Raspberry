#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QString>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QSettings>
#include <QtMath>

#ifndef MILES_PER_KM
# define MILES_PER_KM 0.621371 // More accurate conversion factor
#endif

class CarDataController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int speedLimit READ speedLimit NOTIFY speedLimitChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(int batteryRange READ batteryRange NOTIFY batteryRangeChanged)
    Q_PROPERTY(bool motorActive READ motorActive NOTIFY motorActiveChanged)
    Q_PROPERTY(int motorPower READ motorPower NOTIFY motorPowerChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(double totalDistance READ totalDistance NOTIFY totalDistanceChanged)
    Q_PROPERTY(QString distanceUnit READ distanceUnit WRITE setDistanceUnit NOTIFY distanceUnitChanged)
    Q_PROPERTY(QString temperatureUnit READ temperatureUnit WRITE setTemperatureUnit NOTIFY temperatureUnitChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(bool showError READ showError NOTIFY showErrorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit CarDataController(QObject *parent = nullptr);
    ~CarDataController();
    
    int speed() const;
    int speedLimit() const;
    double totalDistance() const;
    double temperature() const;
    int batteryLevel() const { return m_batteryLevel; }
    int batteryRange() const { return m_batteryRange; }
    bool motorActive() const { return m_motorActive; }
    int motorPower() const { return m_motorPower; }
    QString distanceUnit() const { return m_distanceUnit; }
    QString temperatureUnit() const { return m_temperatureUnit; }
    bool isConnected() const { return m_isConnected; }
    QString serverAddress() const { return m_serverAddress; }
    int serverPort() const { return m_serverPort; }
    bool showError() const { return m_showError; }
    QString errorMessage() const { return m_errorMessage; }

    void setServerAddress(const QString &address);
    void setServerPort(int port);
    void setDistanceUnit(const QString &unit);
    void setTemperatureUnit(const QString &unit);

#ifdef UNIT_TEST
    // Test-only cases setters/updaters
    void testSetTemperature(double value) { updateTemperature(value); };
	void testSetDistance(double value) { updateTotalDistance(value); }
	void testSetSpeed(int value) { updateSpeed(value); }
	void testSetSpeedLimit(int value) { updateSpeedLimit(value); }
	void updateTestBatteryLevel(int value) { updateBatteryLevel(value); }
	void updateTestBatteryRange(int value) { updateBatteryRange(value); }
	void updateTestMotorActive(bool value) { updateMotorActive(value); }
	void updateTestMotorPower(int value) { updateMotorPower(value); }
	void updateTestShowError(bool value) { updateShowError(value); }
	void updateTestErrorMessage(const QString &value) { updateErrorMessage(value); }
	// End of test-only setters
	// Expose internal state for testing
	void testOnReadyRead() { onReadyRead(); }
	void testAttemptReconnect() { attemptReconnect(); }
#endif

public slots:
    void connectToServer();
    void disconnectFromServer();
    void dismissError();
    
signals:
    void speedChanged();
    void speedLimitChanged();
    void batteryLevelChanged();
    void batteryRangeChanged();
    void motorActiveChanged();
    void motorPowerChanged();
    void temperatureChanged();
    void totalDistanceChanged();
    void distanceUnitChanged();
    void temperatureUnitChanged();
    void connectionChanged();
    void serverAddressChanged();
    void serverPortChanged();
    void showErrorChanged();
    void errorMessageChanged();
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void attemptReconnect();

private:
    void updateSpeed(int value);
    void updateSpeedLimit(int value);
    void updateBatteryLevel(int value);
    void updateBatteryRange(int value);
    void updateMotorActive(bool value);
    void updateMotorPower(int value);
    void updateTemperature(double value);
    void updateTotalDistance(double value);
    void updateShowError(bool value);
    void updateErrorMessage(const QString &value);
    
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
    double m_temperature;
    double m_totalDistance;
    QString m_distanceUnit;
    QString m_temperatureUnit;
    bool m_showError;
    QString m_errorMessage;
    
    QString m_buffer;

};
