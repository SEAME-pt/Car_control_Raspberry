#!/usr/bin/env python3
"""
Test server that broadcasts random car data to connected Dashboard clients over TCP.
Messages are encoded in a QDataStream-compatible format (Qt_5_15).

Keyboard commands (press Enter after key):
  e - Trigger error overlay (broadcast error with random message)
  c - Clear error overlay
  q - Quit server
"""

import socket
import struct
import time
import random
import sys
import threading
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
            # QString format: 4-byte length (in bytes) + data
            length = len(encoded)
            self.buffer.extend(struct.pack('>I', length))
            self.buffer.extend(encoded)

    def write_int32(self, value):
        """Write qint32 in QDataStream format"""
        self.buffer.extend(struct.pack('>i', int(value)))

    def write_bool(self, value):
        """Write bool in QDataStream format"""
        self.buffer.extend(struct.pack('B', 1 if value else 0))

    def write_double(self, value):
        """Write double in QDataStream format"""
        self.buffer.extend(struct.pack('>d', float(value)))

    def flush(self):
        """Send all buffered data"""
        try:
            self.sock.sendall(bytes(self.buffer))
        finally:
            self.buffer.clear()


class BroadcastServer:
    def __init__(self, host='127.0.0.1', port=8888):
        self.host = host
        self.port = port
        self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_sock.bind((self.host, self.port))
        self.server_sock.listen(5)
        self.server_sock.settimeout(1.0)

        self.clients = []  # list of (socket, addr)
        self.clients_lock = threading.Lock()
        self.running = False

    def start(self):
        self.running = True
        self.accept_thread = threading.Thread(target=self._accept_loop, daemon=True)
        self.accept_thread.start()
        self.broadcast_thread = threading.Thread(target=self._broadcast_loop, daemon=True)
        self.broadcast_thread.start()
        print(f"Server listening on {self.host}:{self.port}")

    def stop(self):
        self.running = False
        self.accept_thread.join()
        self.broadcast_thread.join()
        with self.clients_lock:
            for c, _ in list(self.clients):
                try:
                    c.shutdown(socket.SHUT_RDWR)
                except Exception:
                    pass
                c.close()
            self.clients.clear()
        try:
            self.server_sock.close()
        except Exception:
            pass
        print("Server stopped")

    def _accept_loop(self):
        while self.running:
            try:
                client_sock, addr = self.server_sock.accept()
                print(f"Client connected: {addr}")
                client_sock.setblocking(True)
                with self.clients_lock:
                    self.clients.append((client_sock, addr))
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"Accept error: {e}")
                break

    def _broadcast_loop(self):
        while self.running:
            # Build random data packet
            speed = random.randint(0, 140)
            speed_limit = random.choice([30, 50, 80, 120])
            battery_level = random.randint(20, 100)
            battery_voltage = random.randint(44, 52)
            battery_range = random.randint(50, 400)
            motor_active = random.choice([True, False])
            motor_power = random.randint(0, 100) if motor_active else 0
            temperature = random.uniform(15.0, 45.0)
            total_km = random.uniform(0, 120000)

            with self.clients_lock:
                for sock, addr in list(self.clients):
                    try:
                        writer = QtDataStreamWriter(sock)
                        writer.write_string("speed")
                        writer.write_int32(speed)

                        writer.write_string("speedLimit")
                        writer.write_int32(speed_limit)

                        writer.write_string("batteryLevel")
                        writer.write_int32(battery_level)

                        writer.write_string("batteryRange")
                        writer.write_int32(battery_range)

                        writer.write_string("motorActive")
                        writer.write_bool(motor_active)

                        writer.write_string("motorPower")
                        writer.write_int32(motor_power)

                        writer.write_string("temperature")
                        writer.write_double(temperature)

                        writer.write_string("totalDistance")
                        writer.write_double(total_km)

                        writer.flush()

                    except (BrokenPipeError, ConnectionResetError, OSError) as e:
                        print(f"Client disconnected {addr}: {e}")
                        try:
                            sock.close()
                        except Exception:
                            pass
                        self.clients.remove((sock, addr))
                    except Exception as e:
                        print(f"Send error to {addr}: {e}")

            print(f"Broadcasted: speed={speed} battery={battery_level}% motor={'ON' if motor_active else 'OFF'} temp={temperature:.1f}km={total_km:.1f}")
            time.sleep(0.5)

    def broadcast_error(self, show_error, message=""):
        with self.clients_lock:
            for sock, addr in list(self.clients):
                try:
                    writer = QtDataStreamWriter(sock)
                    writer.write_string("showError")
                    writer.write_bool(show_error)
                    if message:
                        writer.write_string("errorMessage")
                        writer.write_string(message)
                    writer.flush()
                except Exception as e:
                    print(f"Error broadcasting to {addr}: {e}")
                    try:
                        sock.close()
                    except Exception:
                        pass
                    self.clients.remove((sock, addr))


def main():
    HOST = '127.0.0.1'
    PORT = 8888

    server = BroadcastServer(HOST, PORT)
    server.start()

    print("\n--- Keyboard Commands ---")
    print("e - Trigger error overlay")
    print("c - Clear error overlay")
    print("q - Quit")
    print("-------------------------\n")

    try:
        running = True
        while running:
            # Non-blocking check for keyboard input
            if select.select([sys.stdin], [], [], 0)[0]:
                cmd = sys.stdin.readline().strip().lower()
                if cmd == 'e':
                    error_messages = [
                        "Critical System Error",
                        "Connection Lost",
                        "Battery Warning",
                        "Motor Malfunction",
                        "CAN Bus Error"
                    ]
                    msg = random.choice(error_messages)
                    print(f"Broadcasting error: {msg}")
                    server.broadcast_error(True, msg)
                elif cmd == 'c':
                    print("Clearing error overlay")
                    server.broadcast_error(False, "")
                elif cmd == 'q':
                    print("Quitting server...")
                    running = False
                    break
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    finally:
        server.stop()


if __name__ == "__main__":
    main()
