#ifndef CAR_DATA_COMMUNICATION_HPP
#define CAR_DATA_COMMUNICATION_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <QVariant>

class CarDataCommunication : public QObject {
    Q_OBJECT
    Q_PROPERTY(int cruiseControl READ getCruiseControl WRITE setCruiseControl NOTIFY cruiseControlChanged)

public:
    // Server mode: pass port > 0. Client mode: pass 0 and use connectToHost()
    explicit CarDataCommunication(quint16 port = 0, QObject *parent = nullptr);

    // Client connection API
    void connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();

    // Q_PROPERTY accessors
    int getCruiseControl() const { return speed; }
    void setCruiseControl(int value);

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
    void sendData();


signals:
    // Emitted when a field is received from remote peer
    void fieldReceived(const QString &fieldName, const QVariant &value);
    void cruiseControlChanged();
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onSocketError();

private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    QTcpSocket *clientSocket;
    QTimer *timer;
    bool isServerMode;
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