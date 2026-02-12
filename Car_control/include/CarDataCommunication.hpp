
#pragma once

/**
 * @file include/CarDataCommunication.hpp
 * @brief Qt TCP data stream interface for sending/receiving car telemetry and commands.
 *
 * This header declares the `CarDataCommunication` class which provides a small
 * TCP server/client built on Qt networking classes. It is intended to be used
 * by the car control process to publish telemetry (speed, battery, errors)
 * and receive remote commands (e.g., cruise control adjustments).
 */

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <QVariant>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

typedef struct s_carControl t_carControl;
typedef struct s_CANReceiver t_CANReceiver;

/**
 * @class CarDataCommunication
 * @brief Manages a TCP connection to send and receive car data.
 *
 * The class exposes a small API to update telemetry values and notify
 * connected peers. It is a QObject so it can emit Qt signals when data
 * arrives or the connection status changes.
 */
class CarDataCommunication : public QObject {
    Q_OBJECT
    Q_PROPERTY(int cruiseControl READ getCruiseControl WRITE setCruiseControl NOTIFY cruiseControlChanged)

public:
    /**
     * @brief Construct a CarDataCommunication instance.
     * @param port If >0, starts a TCP server on this port; if 0, instance can act as a client.
     * @param parent QObject parent (optional)
     */
    explicit CarDataCommunication(quint16 port = 0, QObject *parent = nullptr);

    /** @name Client connection API */
    //@{
    /** @brief Connect to a remote host. */
    void connectToHost(const QString &host, quint16 port);
    /** @brief Disconnect from the current host. */
    void disconnectFromHost();
    //@}

    /** @name Q_PROPERTY accessors */
    //@{
    int getCruiseControl() const { return speed; }
    void setCruiseControl(int value);
    //@}

    /**
     * @brief Update the full telemetry payload used for sending.
     *
     * Fields are validated and stored; call `sendData()` to transmit.
     */
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
    /** @brief Update the error message to send to peers. */
    void updateErrorMessage(const QString &value);
    /** @brief Serialize and send current telemetry to connected peer(s). */
    void sendData();


signals:
    // Emitted when a field is received from remote peer
    /** @brief Emitted when an individual field is received from the peer. */
    void fieldReceived(const QString &fieldName, const QVariant &value);
    /** @brief Emitted when the cruise control property changes. */
    void cruiseControlChanged();
    /** @brief Emitted when a connection is established. */
    void connected();
    /** @brief Emitted when a connection is lost. */
    void disconnected();
    /** @brief Emitted on socket or protocol errors. */
    void errorOccurred(const QString &error);

private slots:
    /** @brief Handle new incoming TCP connection (server mode). */
    void onNewConnection();
    /** @brief Handle a client socket disconnecting (server mode). */
    void onClientDisconnected();
    /** @brief Handle incoming data on a socket. */
    void onReadyRead();
    /** @brief Emitted/handled when the client socket connects (client mode). */
    void onConnected();
    /** @brief Emitted/handled when the client socket disconnects (client mode). */
    void onDisconnected();
    /** @brief Handle socket errors and emit `errorOccurred`. */
    void onSocketError();

private:
    QTcpServer *server; /**< TCP server used in server mode (owns client sockets). */
    QList<QTcpSocket*> clients; /**< Client sockets connected to the server (server mode). */
    QTcpSocket *clientSocket; /**< Single client socket used in client mode. */
    QTimer *timer; /**< Timer used to trigger periodic sends. */
    bool isServerMode; /**< True when the instance is running as a server. */
    // Data to send
    int speed; /**< Current speed (also used for `cruiseControl` property). */
    int speedLimit; /**< Configured speed limit. */
    int batteryLevel; /**< Battery percentage (0-100). */
    int batteryRange; /**< Estimated battery range in kilometers. */
    bool motorActive; /**< Whether the motor is active. */
    int motorPower; /**< Motor output power (arbitrary units). */
    double temperature; /**< Temperature in degrees Celsius. */
    double totalDistance; /**< Total traveled distance in kilometers. */
    bool showError; /**< Whether an error should be reported to peers. */
    QString errorMessage; /**< Optional human-readable error message. */
};

/**
 * @brief Thread entry used by the Qt data-stream helper.
 *
 * This free function is intended to be launched in a separate thread and
 * will create a `QCoreApplication`/event loop and manage `CarDataCommunication`.
 */

/**
 * @brief Thread entry used by the Qt data-stream helper.
 *
 * Declared here so `main.cpp` (and other users) can spawn the thread without
 * including the qDataStream implementation file directly.
 */
void	qDataStreamThread(t_carControl* carControl, t_CANReceiver* canReceiver, int argc, char *argv[]);
