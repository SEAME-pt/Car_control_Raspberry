# Socket Communication Protocol - QDataStream Implementation

## Overview

This document describes the TCP/IP socket communication protocol used between the **Car_control** (transmitting end) and **Dashboard** (receiving end) applications. The communication uses Qt's `QDataStream` for binary serialization, providing efficient and type-safe data transmission.

## Architecture

### Car_control (Transmitting End)
- **Component**: `CarDataSender` class
- **Location**: `/Car_control/srcs/communication/`
- **Role**: TCP Server that broadcasts car telemetry data to connected clients
- **Port**: 8888 (default)
- **Protocol**: Qt Binary Protocol using `QDataStream`

### Dashboard (Receiving End)
- **Component**: `CarDataController` class
- **Location**: `/Dashboard/srcs/` and `/Dashboard/incs/`
- **Role**: TCP Client that receives and processes car telemetry data
- **Protocol**: Qt Binary Protocol using `QDataStream`

## Connection Details

### Server (Car_control)
```cpp
// Server configuration
QTcpServer listening on: 0.0.0.0:8888 (any interface)
Update frequency: 500ms (2 Hz)
Protocol: QDataStream Qt_5_15
```

### Client (Dashboard)
```cpp
// Client configuration
Server address: 127.0.0.1 (configurable)
Server port: 8888 (configurable)
Auto-reconnect: Every 5 seconds on disconnect
Protocol: QDataStream Qt_5_15
```

## Data Format

### QDataStream Configuration
Both ends use:
```cpp
QDataStream::setVersion(QDataStream::Qt_5_15);
```

This ensures compatibility and consistent serialization across Qt versions.

### Message Structure

Each message consists of multiple field-value pairs serialized sequentially:

```
[field_name (QString)][value (Type)] [field_name (QString)][value (Type)] ...
```

### Field Definitions

| Field Name      | C++ Type  | Description                          | Unit/Range      |
|----------------|-----------|--------------------------------------|-----------------|
| `speed`        | `qint32`  | Current vehicle speed                | km/h            |
| `speedLimit`   | `qint32`  | Speed limit setting                  | km/h            |
| `batteryLevel` | `qint32`  | Battery charge percentage            | 0-100 %         |
| `batteryRange` | `qint32`  | Estimated remaining range            | km              |
| `motorActive`  | `bool`    | Motor state (on/off)                 | true/false      |
| `motorPower`   | `qint32`  | Motor power output                   | 0-100 %         |
| `temperature`  | `double`  | System temperature (optional)        | °C              |
| `totalDistance` | `double` | Total distance traveled (optional)   | km              |
| `showError`    | `bool`    | Error display flag (optional)        | true/false      |
| `errorMessage` | `QString` | Error message text (optional)        | UTF-8 string    |

## Implementation Details

### Transmitting End (Car_control)

#### CarDataSender Class

**Header**: `Car_control/srcs/communication/CarDataSender.hpp`

```cpp
class CarDataSender : public QObject {
    Q_OBJECT
public:
    CarDataSender(quint16 port, QObject *parent = nullptr);
    void updateData(int speed, int speedLimit, int batteryLevel, 
                   int batteryRange, bool motorActive, int motorPower);
private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;
    QTimer *timer;
    // Data members...
};
```

**Implementation**: `Car_control/srcs/communication/CarDataSender.cpp`

#### Data Transmission Logic

```cpp
void CarDataSender::sendData() {
    if (clients.isEmpty()) {
        return;
    }

    // Prepare data using QDataStream
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    // Serialize field-value pairs
    out << QString("speed") << (qint32)speed;
    out << QString("speedLimit") << (qint32)speedLimit;
    out << QString("batteryLevel") << (qint32)batteryLevel;
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
```

### Receiving End (Dashboard)

#### CarDataController Class

**Header**: `Dashboard/incs/CarDataController.hpp`

```cpp
class CarDataController : public QObject {
    Q_OBJECT
    // Q_PROPERTY declarations for QML binding...
public:
    explicit CarDataController(QObject *parent = nullptr);
    // Getters for all data fields...
    
public slots:
    void connectToServer();
    void disconnectFromServer();
    void reconnect();
    
private slots:
    void onReadyRead();
    // Other connection management slots...
    
private:
    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;
    // Data members...
};
```

**Implementation**: `Dashboard/srcs/CarDataController.cpp`

#### Data Reception Logic

```cpp
void CarDataController::onReadyRead() {
    // Read data using QDataStream
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);
    
    while (m_socket->bytesAvailable() > 0) {
        // Read field identifier
        QString field;
        in >> field;
        
        // Parse based on field name
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
        }
        // Additional fields...
        
        // Error checking
        if (in.status() != QDataStream::Ok) {
            qWarning() << "Error reading data stream";
            break;
        }
    }
}
```

**Key Features**:
- Automatic reconnection on disconnect (5-second interval)
- Stream error detection and handling
- Signal-based updates for reactive UI
- Unit conversion support (km/miles, Celsius/Fahrenheit)

## Usage Examples

### Initializing the Sender (Car_control)

```cpp
#include "CarDataSender.hpp"

int main() {
    QCoreApplication app(argc, argv);
    
    // Create sender on port 8888
    CarDataSender sender(8888);
    
    // Update data periodically (e.g., from CAN bus)
    sender.updateData(
        60,    // speed (km/h)
        120,   // speedLimit (km/h)
        85,    // batteryLevel (%)
        267,   // batteryRange (km)
        true,  // motorActive
        75     // motorPower (%)
    );
    
    return app.exec();
}
```

### Initializing the Receiver (Dashboard)

```cpp
#include "CarDataController.hpp"

int main() {
    QGuiApplication app(argc, argv);
    
    // Create controller (auto-connects on startup)
    CarDataController controller;
    
    // Optional: Configure server details
    controller.setServerAddress("192.168.1.100");
    controller.setServerPort(8888);
    controller.connectToServer();
    
    // Access data via getters or Q_PROPERTY bindings
    int speed = controller.speed();
    
    return app.exec();
}
```

## Error Handling

### Connection Errors

Both ends handle connection issues gracefully:

**Sender**:
- Continues operation even with no clients connected
- Automatically removes disconnected clients from broadcast list

**Receiver**:
- Attempts automatic reconnection every 5 seconds
- Emits `errorOccurred(QString)` signal on socket errors
- Updates `isConnected` property for UI feedback

### Stream Errors

```cpp
if (in.status() != QDataStream::Ok) {
    qWarning() << "Error reading data stream";
    break;  // Stop processing corrupted stream
}
```

## Testing

### Test Data Script

A Python test script is available at: `scripts/send_test_data.py`

This script can simulate the Car_control sender for testing purposes.

## Best Practices

### For Transmitters (Car_control)

1. **Always flush after writing**:
   ```cpp
   client->write(data);
   client->flush();
   ```

2. **Check connection state before sending**:
   ```cpp
   if (client->state() == QAbstractSocket::ConnectedState) {
       client->write(data);
   }
   ```

3. **Use explicit type casting**:
   ```cpp
   out << (qint32)speed;  // Ensure consistent type
   ```

### For Receivers (Dashboard)

1. **Match QDataStream version**:
   ```cpp
   in.setVersion(QDataStream::Qt_5_15);
   ```

2. **Check stream status after reading**:
   ```cpp
   if (in.status() != QDataStream::Ok) {
       // Handle error
   }
   ```

3. **Process all available data**:
   ```cpp
   while (m_socket->bytesAvailable() > 0) {
       // Read fields
   }
   ```

## Protocol Extension

### Adding New Fields

To add a new field to the protocol:

1. **Update Sender** (`CarDataSender.cpp`):
   ```cpp
   out << QString("newField") << (qint32)newValue;
   ```

2. **Update Receiver** (`CarDataController.cpp`):
   ```cpp
   else if (field == "newField") {
       qint32 value;
       in >> value;
       updateNewField(value);
   }
   ```

3. **Add corresponding members and signals** to both classes

### Backward Compatibility

The protocol maintains backward compatibility:
- Unknown fields are ignored by the receiver
- Missing fields retain previous values
- Both ends can operate with different field sets

## Performance Considerations

- **Binary Protocol**: More efficient than JSON/XML text protocols
- **Update Rate**: 500ms provides smooth updates without excessive bandwidth
- **Data Size**: Typical message size ~200-300 bytes
- **Network Overhead**: Minimal due to binary serialization

## Troubleshooting

### Common Issues

1. **Connection Refused**:
   - Ensure Car_control server is running
   - Check firewall settings
   - Verify correct IP address and port

2. **Data Not Updating**:
   - Check `isConnected` property
   - Verify QDataStream version matches
   - Check for stream errors in logs

3. **Partial Data Received**:
   - Ensure all fields are sent in correct order
   - Verify type casting is consistent
   - Check for network interruptions

### Debug Logging

Enable debug output:
```cpp
qDebug() << "Speed:" << speed;
qDebug() << "Stream status:" << in.status();
qDebug() << "Bytes available:" << socket->bytesAvailable();
```

## References

- Qt Documentation: [QDataStream](https://doc.qt.io/qt-5/qdatastream.html)
- Qt Documentation: [QTcpSocket](https://doc.qt.io/qt-5/qtcpsocket.html)
- Qt Documentation: [QTcpServer](https://doc.qt.io/qt-5/qtcpserver.html)
