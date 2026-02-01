#!/usr/bin/env python3
"""
C++ QDataStream compatible server for car data.
This demonstrates the protocol - for production, write this in C++.
"""

import socket
import struct
import time
import random
import math

HOST = '127.0.0.1'
PORT = 8888

def write_qstring(data, s):
    """Write a QString in Qt's QDataStream format."""
    # QString: 4 bytes length (uint32) + UTF-16 encoded string
    encoded = s.encode('utf-16-be')
    data += struct.pack('>I', len(encoded))
    data += encoded
    return data

def write_int32(data, value):
    """Write a qint32."""
    return data + struct.pack('>i', value)

def write_bool(data, value):
    """Write a bool."""
    return data + struct.pack('>?', value)

def send_car_data(sock, t):
    """Send car data using QDataStream protocol."""
    # Create a pattern with clear acceleration and braking phases
    cycle = t % 40
    
    if cycle < 20:
        # Accelerating phase
        speed = int((cycle / 20) * 100)
        motor_power = min(100, int((cycle / 20) * 100) + 30)
    else:
        # Braking phase
        speed = int(((40 - cycle) / 20) * 100)
        motor_power = max(-100, -int(((cycle - 20) / 20) * 80))
    
    # Battery drains more noticeably
    battery_level = max(20, 100 - int(t / 2))
    
    # Voltage changes slightly with load
    battery_voltage = int(48 + random.uniform(-2, 2))
    
    # Range based on battery level
    battery_range = int(battery_level * 3)
    
    # Motor is active when moving
    motor_active = speed > 0 or abs(motor_power) > 0
    
    # Speed limit varies
    speed_limits = [30, 50, 80, 100, 120]
    speed_limit = speed_limits[int(t / 20) % len(speed_limits)]
    
    # Build QDataStream message
    data = b''
    
    # Send speed
    data = write_qstring(data, "speed")
    data = write_int32(data, speed)
    
    # Send speedLimit
    data = write_qstring(data, "speedLimit")
    data = write_int32(data, speed_limit)
    
    # Send batteryLevel
    data = write_qstring(data, "batteryLevel")
    data = write_int32(data, battery_level)
    
    # Send batteryVoltage
    data = write_qstring(data, "batteryVoltage")
    data = write_int32(data, battery_voltage)
    
    # Send batteryRange
    data = write_qstring(data, "batteryRange")
    data = write_int32(data, battery_range)
    
    # Send motorActive
    data = write_qstring(data, "motorActive")
    data = write_bool(data, motor_active)
    
    # Send motorPower
    data = write_qstring(data, "motorPower")
    data = write_int32(data, motor_power)
    
    sock.sendall(data)
    print(f"Sent: speed={speed}, battery={battery_level}%, range={battery_range}km, motor={motor_power}")

def handle_client(conn, addr):
    """Handle a connected client."""
    print(f"Connected by {addr}")
    t = 0
    
    try:
        while True:
            send_car_data(conn, t)
            t += 1
            time.sleep(0.5)
    except (BrokenPipeError, ConnectionResetError):
        print("Client disconnected")
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        conn.close()

def main():
    """Start the TCP server."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        
        print(f"Car data server (QDataStream protocol) listening on {HOST}:{PORT}")
        print("Waiting for dashboard connection...")
        print("Press Ctrl+C to stop")
        
        try:
            while True:
                conn, addr = s.accept()
                handle_client(conn, addr)
        except KeyboardInterrupt:
            print("\nServer stopped")

if __name__ == '__main__':
    main()
