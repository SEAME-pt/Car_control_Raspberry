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

    void updateData(int speed, int speedLimit, int batteryLevel, int batteryVoltage, int batteryRange, bool motorActive, int motorPower);

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
    int batteryVoltage;
    int batteryRange;
    bool motorActive;
    int motorPower;
};

#endif // CAR_DATA_SENDER_HPP