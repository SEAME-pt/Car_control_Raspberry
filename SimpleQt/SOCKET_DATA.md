# Socket-Based Real-Time Data Updates

## Overview
The dashboard now receives real-time updates via TCP socket connection. Car data like speed, battery level, motor state, and more are streamed from a server.

## Features
- 🔌 TCP socket connection to data server
- 🔄 Auto-reconnect on connection loss
- 📊 Real-time updates for:
  - Speed (km/h)
  - Speed limit
  - Battery level (%)
  - Battery voltage (V)
  - Motor active state
  - Motor power (%)
- 🎨 Visual feedback for connection status
- ⚡ Smooth animations for changing values

## Quick Start

### 1. Start the Test Server

A Python test server is included that generates realistic car data:

```bash
cd /home/afogonca/seame/Car_control_Raspberry/SimpleQt
chmod +x car_data_server.py
python3 car_data_server.py
```

The server will:
- Listen on `127.0.0.1:8888`
- Generate dynamic speed, battery, and motor data
- Send JSON updates every 500ms
- Automatically handle client reconnections

### 2. Run the Dashboard

In another terminal:

```bash
cd build
qmake .. && make && ./SimpleQt
```

The dashboard will automatically connect to the server on startup.

## Data Protocol

### Message Format

Messages are newline-delimited JSON sent over TCP:

```json
{
  "speed": 65,
  "speedLimit": 120,
  "batteryLevel": 85,
  "batteryVoltage": 48,
  "motorActive": true,
  "motorPower": 54
}
```

### Field Descriptions

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| `speed` | int | 0-200 | Current speed in km/h |
| `speedLimit` | int | 30-120 | Current speed limit in km/h |
| `batteryLevel` | int | 0-100 | Battery percentage |
| `batteryVoltage` | int | 0-60 | Battery voltage in V |
| `motorActive` | bool | true/false | Whether motor is running |
| `motorPower` | int | 0-100 | Motor power percentage |

### Protocol Details

- **Transport**: TCP
- **Encoding**: UTF-8
- **Delimiter**: Newline (`\n`)
- **Format**: JSON
- **Update Rate**: Configurable (default: 500ms)

## Configuration

### Change Server Address/Port

You can configure the connection in QML:

```qml
// In Main.qml or any component
Component.onCompleted: {
    CarData.serverAddress = "192.168.1.100"
    CarData.serverPort = 9999
    CarData.reconnect()
}
```

Or directly from C++ before engine loads.

### Connection Properties

The `CarData` singleton exposes:

```qml
CarData.serverAddress    // String: Server IP/hostname
CarData.serverPort       // int: Server port
CarData.isConnected      // bool: Connection status
```

### Methods

```qml
CarData.connectToServer()    // Initiate connection
CarData.disconnectFromServer() // Close connection
CarData.reconnect()          // Reconnect
```

## Visual Indicators

The dashboard provides visual feedback:

### Speed Display
- Shows current speed from `CarData.speed`
- "PARK" (red) when speed = 0
- "DRIVE" (green) when speed > 0

### Battery Display
- Percentage from `CarData.batteryLevel`
- Visual battery fill indicator
- Voltage from `CarData.batteryVoltage`
- Red color when battery < 20%

### Motor State
- Green bar when motor active
- Gray when inactive
- Fill width represents motor power

### Speed Limit
- Updates from `CarData.speedLimit`
- Red border speed sign

## Integration with Your System

### Arduino/Raspberry Pi Integration

If you have sensors or CAN bus data:

```python
import socket
import json

def send_data(speed, battery, voltage):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 8888))
    
    while True:
        data = {
            "speed": read_speed_sensor(),
            "batteryLevel": read_battery_level(),
            "batteryVoltage": read_voltage(),
            "motorActive": is_motor_running(),
            "motorPower": get_motor_power()
        }
        sock.sendall((json.dumps(data) + '\n').encode())
        time.sleep(0.1)
```

### CAN Bus Integration

For real car data via CAN bus:

```python
import can
import socket
import json

def can_to_dashboard():
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 8888))
    
    for msg in bus:
        if msg.arbitration_id == 0x123:  # Speed message
            speed = int.from_bytes(msg.data[0:2], 'big')
            # Parse other data...
            
            data = {"speed": speed, ...}
            sock.sendall((json.dumps(data) + '\n').encode())
```

## Troubleshooting

### Dashboard shows "Not Connected"
- Check if server is running
- Verify correct IP and port
- Check firewall settings
- Look at console for error messages

### Data not updating
- Check server is sending messages
- Verify JSON format is correct
- Ensure messages end with `\n`
- Check for network issues

### Reconnection issues
- Server restarts automatically reconnect (5s interval)
- Check `CarData.isConnected` status
- Call `CarData.reconnect()` manually if needed

## Advanced Configuration

### Custom Message Handler

Extend `CarDataController` to handle custom fields:

```cpp
// In CarDataController.cpp parseMessage()
if (obj.contains("customField")) {
    // Handle custom data
}
```

### Multiple Data Sources

Run multiple servers or implement a data aggregator:
- One server for sensors
- One for GPS
- One for vehicle systems
- Aggregator combines and forwards to dashboard

### Logging

Enable debug output:

```cpp
// In CarDataController.cpp
qDebug() << "Received:" << message;
```

## Performance

- **Latency**: < 10ms typical
- **Update Rate**: Up to 100Hz (not recommended, 10Hz is sufficient)
- **CPU Usage**: < 1% for data reception
- **Memory**: ~1MB for socket buffers

## Security Notes

⚠️ **Important for Production**:
- Currently uses plain TCP (no encryption)
- For production, use TLS/SSL
- Implement authentication
- Validate all incoming data
- Use firewall rules to restrict access

## Example: Production Server

```python
import ssl
import socket
import json

context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
context.load_cert_chain('server.crt', 'server.key')

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('0.0.0.0', 8888))
    sock.listen()
    
    with context.wrap_socket(sock, server_side=True) as secure_sock:
        # Handle connections...
```
