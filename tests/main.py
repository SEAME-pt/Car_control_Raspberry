# virtual_gamepad.py
from evdev import UInput, ecodes as e, AbsInfo
import time
import os
import signal
import sys

def signal_handler(sig, frame):
    print('\nCleaning up virtual gamepad...')
    try:
        os.remove("/tmp/virtual_gamepad_path.txt")
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

ui = UInput(caps, name="VirtualPhoneGamepad", version=1)

# Write device path to file for C++ to read
device_path = ui.device.path
with open("/tmp/virtual_gamepad_path.txt", "w") as f:
    f.write(device_path)

try:
    while True:
        if sys.argv[1] == "A":
            # Simulate pressing the A button
            ui.write(e.EV_KEY, e.BTN_SOUTH, 1); ui.syn()
            time.sleep(0.1)
            ui.write(e.EV_KEY, e.BTN_SOUTH, 0); ui.syn()
            time.sleep(0.4)
        else:
            # Simulate pressing the B button
            ui.write(e.EV_KEY, e.BTN_EAST, 1); ui.syn()
            time.sleep(0.1)
            ui.write(e.EV_KEY, e.BTN_EAST, 0); ui.syn()
            time.sleep(0.4)
        
except KeyboardInterrupt:
    pass
finally:
    ui.close()
    try:
        os.remove("/tmp/virtual_gamepad_path.txt")
    except:
        pass
