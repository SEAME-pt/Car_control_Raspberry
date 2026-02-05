#ifndef CAR_DATA_SENDER_HPP
#define CAR_DATA_SENDER_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>

class CarDataSender : public QObject {
    Q_OBJECT
public:
    CarDataSender(quint16 port, QObject *parent = nullptr);

    void updateData(int speed, int speedLimit, int batteryLevel, int batteryRange, 
                   bool motorActive, int motorPower, double temperature = 20.0, 
                   double totalDistance = 0.0, bool showError = false, 
                   const QString &errorMessage = "");

    // Individual update functions
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

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void sendData();

private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    QTimer *timer;

    // Data to send
    int speed;
    int speedLimit;
    int batteryLevel;
    int batteryRange;
    bool motorActive;
    int motorPower;
    double temperature;
    double totalDistance;
    bool showError;
    QString errorMessage;
};

#endif // CAR_DATA_SENDER_HPP