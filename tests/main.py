# virtual_gamepad.py
from evdev import UInput, ecodes as e, AbsInfo
import time
import os
import signal
import sys
from unittest.mock import Mock

def signal_handler(sig, frame):
    print('\nCleaning up virtual gamepad...')
    try:
        os.remove(os.path.expanduser("~/virtual_gamepad_path.txt"))
    except:
        pass
    try:
        # Clean up mock pipe/file
        if os.path.exists("/tmp/mock_gamepad_fifo"):
            os.remove("/tmp/mock_gamepad_fifo")
        if os.path.exists("/tmp/mock_gamepad_file"):
            os.remove("/tmp/mock_gamepad_file")
    except:
        pass
    sys.exit(0)

# Register signal handler for cleanup
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

caps = {
    e.EV_KEY: [e.BTN_SOUTH, e.BTN_EAST, e.BTN_NORTH, e.BTN_WEST, e.BTN_TL, e.BTN_TR],
    e.EV_ABS: {
        e.ABS_X: AbsInfo(0, -32768, 32767, 0, 0, 0),
        e.ABS_Y: AbsInfo(0, -32768, 32767, 0, 0, 0)
    }
}

# Use mock for testing by default, unless REAL_GAMEPAD=true
use_real_gamepad = os.getenv('REAL_GAMEPAD', 'false').lower() == 'true'

try:
    if use_real_gamepad:
        ui = UInput(caps, name="VirtualPhoneGamepad", version=1)
        device_path = ui.device.path
    else:
        # Mock gamepad for testing
        print("Using mock gamepad for testing")
        ui = Mock()
        ui.device = Mock()
        ui.device.path = "/tmp/mock_gamepad_fifo"
        ui.write = Mock()
        ui.syn = Mock()
        ui.close = Mock()
        device_path = ui.device.path
        
        # Create a named pipe for mock testing
        import os
        import stat
        try:
            if os.path.exists(device_path):
                os.remove(device_path)
            os.mkfifo(device_path)
            # Make it readable/writable
            os.chmod(device_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IWGRP | stat.S_IROTH | stat.S_IWOTH)
        except Exception as e:
            print(f"Failed to create mock pipe: {e}")
            # Fallback to a regular file for basic testing
            device_path = "/tmp/mock_gamepad_file"
            with open(device_path, 'w') as f:
                f.write("")
            ui.device.path = device_path
except PermissionError:
    print("Permission denied! Try one of these solutions:")
    print("1. Add user to input group: sudo usermod -a -G input $USER")
    print("2. Run with sudo (not recommended for tests)")
    print("3. Set up udev rules for /dev/uinput")
    print("4. Use mock mode (default): unset REAL_GAMEPAD or set REAL_GAMEPAD=false")
    sys.exit(1)
except Exception as ex:
    print(f"Failed to create virtual gamepad: {ex}")
    sys.exit(1)

# Write device path to file for C++ to read
gamepad_path_file = os.path.expanduser("~/virtual_gamepad_path.txt")
with open(gamepad_path_file, "w") as f:
    f.write(device_path)

print(f"Virtual gamepad created at: {device_path}")

try:
    while True:
        if sys.argv[1] == "A":
            if use_real_gamepad:
                # Simulate pressing the A button
                ui.write(e.EV_KEY, e.BTN_SOUTH, 1); ui.syn()
                time.sleep(0.1)
                ui.write(e.EV_KEY, e.BTN_SOUTH, 0); ui.syn()
                time.sleep(0.4)
            else:
                # Mock mode - write some data to the pipe/file to simulate activity
                if device_path.endswith("_fifo"):
                    try:
                        with open(device_path, 'wb') as f:
                            # Write a simple byte pattern to simulate some activity
                            f.write(b'\x01\x02\x03\x04')
                            f.flush()
                    except:
                        pass
                time.sleep(0.5)
        else:
            if use_real_gamepad:
                # Simulate pressing the B button
                ui.write(e.EV_KEY, e.BTN_EAST, 1); ui.syn()
                time.sleep(0.1)
                ui.write(e.EV_KEY, e.BTN_EAST, 0); ui.syn()
                time.sleep(0.4)
            else:
                # Mock mode - write different data pattern
                if device_path.endswith("_fifo"):
                    try:
                        with open(device_path, 'wb') as f:
                            f.write(b'\x05\x06\x07\x08')
                            f.flush()
                    except:
                        pass
                time.sleep(0.5)
        
except KeyboardInterrupt:
    pass
finally:
    ui.close()
    try:
        os.remove(os.path.expanduser("~/virtual_gamepad_path.txt"))
    except:
        pass
    try:
        # Clean up mock pipe/file
        if os.path.exists("/tmp/mock_gamepad_fifo"):
            os.remove("/tmp/mock_gamepad_fifo")
        if os.path.exists("/tmp/mock_gamepad_file"):
            os.remove("/tmp/mock_gamepad_file")
    except:
        pass
