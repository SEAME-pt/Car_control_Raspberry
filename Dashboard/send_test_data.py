#!/usr/bin/env python3
"""
Test script to send random car data to the Dashboard via TCP socket.
Uses QDataStream format compatible with Qt 5.15.
Press 'e' + Enter to trigger an error overlay.
Press 'c' + Enter to clear the error.
Press 'q' + Enter to quit.
"""

import socket
import struct
import time
import random
import sys
import select

class QtDataStreamWriter:
    """Helper class to write data in Qt QDataStream format (Qt_5_15)"""
    
    def __init__(self, sock):
        self.sock = sock
        self.buffer = bytearray()
    
    def write_string(self, s):
        """Write QString in QDataStream format"""
        if s is None or s == "":
            # Empty string: write 0 length
            self.buffer.extend(struct.pack('>I', 0))
        else:
            # Encode as UTF-16 BE (big endian)
            encoded = s.encode('utf-16-be')
            # QString format: 4-byte length (in bytes, not characters) + data
            length = len(encoded)
            self.buffer.extend(struct.pack('>I', length))
            self.buffer.extend(encoded)
    
    def write_int32(self, value):
        """Write qint32 in QDataStream format"""
        self.buffer.extend(struct.pack('>i', value))
    
    def write_bool(self, value):
        """Write bool in QDataStream format"""
        self.buffer.extend(struct.pack('B', 1 if value else 0))
    
    def flush(self):
        """Send all buffered data"""
        self.sock.sendall(bytes(self.buffer))
        self.buffer.clear()

def send_random_data(writer):
    """Send random car data"""
    # Randomize values
    speed = random.randint(0, 180)
    speed_limit = random.choice([30, 50, 80, 120])
    battery_level = random.randint(20, 100)
    battery_voltage = random.randint(44, 52)
    battery_range = random.randint(50, 400)
    motor_active = random.choice([True, False])
    motor_power = random.randint(0, 100) if motor_active else 0
    
    # Send all fields
    writer.write_string("speed")
    writer.write_int32(speed)
    
    writer.write_string("speedLimit")
    writer.write_int32(speed_limit)
    
    writer.write_string("batteryLevel")
    writer.write_int32(battery_level)
    
    writer.write_string("batteryVoltage")
    writer.write_int32(battery_voltage)
    
    writer.write_string("batteryRange")
    writer.write_int32(battery_range)
    
    writer.write_string("motorActive")
    writer.write_bool(motor_active)
    
    writer.write_string("motorPower")
    writer.write_int32(motor_power)
    
    # Flush all data at once
    writer.flush()
    
    print(f"Sent: Speed={speed}, Battery={battery_level}%, Motor={'ON' if motor_active else 'OFF'}")

def send_error(writer, show_error, message=""):
    """Send error state"""
    writer.write_string("showError")
    writer.write_bool(show_error)
    
    if message:
        writer.write_string("errorMessage")
        writer.write_string(message)
    
    writer.flush()
    print(f"Sent: showError={show_error}, message='{message}'")

def check_keyboard_input():
    """Check if keyboard input is available (non-blocking)"""
    return select.select([sys.stdin], [], [], 0)[0]

def main():
    HOST = '127.0.0.1'
    PORT = 8888
    
    print(f"Starting TCP server on {HOST}:{PORT}...")
    print("Waiting for Dashboard to connect...")
    
    try:
        # Create server socket
        server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_sock.bind((HOST, PORT))
        server_sock.listen(1)
        
        print(f"Server listening on {HOST}:{PORT}")
        print("Start the Dashboard application now to connect...\n")
        
        # Accept connection from Dashboard
        client_sock, client_addr = server_sock.accept()
        print(f"Dashboard connected from {client_addr}")
        
        writer = QtDataStreamWriter(client_sock)
        
        print("\n--- Keyboard Commands ---")
        print("e - Trigger error overlay")
        print("c - Clear error overlay")
        print("q - Quit")
        print("-------------------------\n")
        print("Sending random data every 0.5 seconds...")
        
        # Main loop - send random data periodically and check for keyboard input
        running = True
        while running:
            try:
                # Send data
                send_random_data(writer)
                
                # Check for keyboard input (non-blocking)
                if check_keyboard_input():
                    user_input = sys.stdin.readline().strip().lower()
                    
                    if user_input == 'e':
                        error_messages = [
                            "Critical System Error",
                            "Connection Lost",
                            "Battery Warning",
                            "Motor Malfunction",
                            "CAN Bus Error"
                        ]
                        message = random.choice(error_messages)
                        send_error(writer, True, message)
                    elif user_input == 'c':
                        send_error(writer, False, "")
                    elif user_input == 'q':
                        print("Quitting...")
                        running = False
                        break
                
                time.sleep(0.5)
            except KeyboardInterrupt:
                print("\nInterrupted by user")
                break
            except BrokenPipeError:
                print("Connection closed by Dashboard")
                break
            except Exception as e:
                print(f"Error sending data: {e}")
                break
        
        client_sock.close()
        server_sock.close()
        print("Connection closed")
        
    except OSError as e:
        print(f"Could not start server on {HOST}:{PORT}")
        print(f"Error: {e}")
        print("Make sure no other process is using this port!")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
