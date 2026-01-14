# TEAM1 - Velocity Era

This repository consolidates the most up to date software stack for our autonomous vehicle, Vera, which runs on a Raspberry Pi 5. Currently, the program fully integrates Joystick support, capturing axis values and converting them messages. These values are then transmitted via CAN-FD to the microcontroller, enabling safe and reliable control of Vera’s movement in a safety critical environment.

Additionally, the program is responsible for running the Qt interface, which displays on the car’s screen the data received from the microcontroller (such as speed sensor readings and motor/servo feedback) also communicated through CAN-FD.

## Overview

Components of this project:
- Raspberry Pi 5
- CAN-BUS(FD) HAT for Raspberry Pi
- Joystick

This platform integrates dependencies like:
- LIBEVDEV
- SocketCAN
- Google Tests
- lcov

---

## File Structure

```
Car_control/
│
├── CMakeLists.txt                       # Build configuration
│
├── include/                             # Header files
│   ├── CANController.hpp                # CAN controller class (smart pointer)
│   ├── CANProtocol.hpp                  # CAN protocol definitions
│   ├── Joystick.hpp 					 # Joystick class definition
│   ├── carControl.h                     # Main control structures and functions
│   ├── exceptions.hpp                   # Custom exception classes
│   └── socketCAN.h                      # C wrapper for CAN socket operations
│
├── srcs/                                # Source files
│   ├── main.cpp                         # Main entry point
│   │
│   ├── can/                          	 # CAN implementation
│   │   ├── CANController.cpp            # CAN controller implementation (RAII)
│   │   └── socketCAN.c                  # Low-level CAN socket operations
│   │
│   ├── controller/                      # Input controllers
│   │   └── Joystick.cpp                 # Joystick handling
│   │
│   ├── core/ 							 # Core control loops
│   │   ├── autonomous_mode.cpp 		 # Autonomous driving loop
│   │   └── manual_mode.cpp 			 # Manual joystick control loop
│   │
│   ├── init/                            # Initialization modules
│   │   ├── init.cpp                     # Main initialization
│   │
│   └── utils/                           # Utility functions
│   |   ├── parsing.cpp 				 # Command-line argument parsing
│   |   ├── signal.cpp 					 # Signal handling (graceful shutdown)
│   |── └── temp_print_can_rx.cpp 		 # CAN receive debug utility
│
├── tests/
│ ├── CANControllerTest.cpp 			 # CAN controller unit tests
│ ├── CANInitTest.cpp 					 # CAN initialization tests
│ ├── CANProtocolTest.cpp 				 # CAN protocol tests
│ ├── CarControlTest.cpp 				 # Main control system tests
│ ├── JoystickTest.cpp 					 # Joystick handling tests
│ └── SocketCANTest.cpp 				 # Low-level CAN socket tests
│
├── scripts/
│ ├── install_lcov.sh 					 # Install lcov for coverage
│ ├── install_GoogleTest.sh 			 # Install Google Test framework
│ └── install_libevdev.sh 				 # Install libevdev library
│
└── build/                               # Build output (gitignored)
    ├── Makefile
    └── car                              # Executable
```

## CAN

Initialization of SocketCAN:
```cpp
// SocketCAN initialization
s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

// Informs Kernel to accept CAN_FD and allows sending and receiving
// CAN frames on the same socket
setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, 
			&enable_canfd, sizeof(enable_canfd));

//bind socket to interface
bind(s, (struct sockaddr *)&addr, sizeof(addr));
```

Create and configure Instructions to send via CAN:
```cpp
struct canfd_frame frame;
memset(&frame, 0, sizeof(frame));

//populate canfd_frame struct
frame.can_id = can_id;
frame.len = len;
frame.flags = CANFD_BRS; // Faster Bit Rate Switch (only for CAN_FD)

//write bits to the desired socket 
write(socket, &frame, sizeof(struct canfd_frame))

// Using the same socket, we can check for incoming messages.
// The use of poll() ensures efficient, non-blocking operation,
// returning immediately when no data is available to read.
if (poll(&pfd, 1, 0) <= 0)
	return (-1);
if (read(socket, frame, sizeof(*frame)) < 0)
	return (-1);
```

CAN instructions and their respective id's are:

Raspberry Pi -> STM32:

```cpp
namespace CANID {
	constexpr uint16_t EMERGENCY_BRAKE = 0x100;		// max priority
	constexpr uint16_t DRIVE_COMMAND   = 0x101;		// high priority
	constexpr uint16_t DRIVE_MODE      = 0x102;		// low priority
};
```

## Joystick

TODO

# Build System

## Instructions to run the program

Is this your first time running the program? If so, check out how to properly install all dependencies:

```shell
cd scripts
chmod +x install_lcov.sh
chmod +x install_GoogleTest.sh
chmod +x install_libevdev.sh

./install_lcov.sh
./install_GoogleTest.sh
./install_libevdev.sh

cd .. && mkdir build && cd build
cmake ..
make
```

After that, all you need to do is use the Makefile and run the executable. But before proceeding, if you have any questions, first run:

```shell
# This will show all available input options, as well as the default values that will be used if none are selected.
./car --help
```

```bash
# Default values
# Notice that default values are prepared ONLY for the final result of communication
# with STM32 and properly connection with Joystick
cd build
cmake ..
make
sudo ./car
```

```shell
# Debug purposes inside coding machine
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 mtu 16 # or 72 if can_fd
sudo ip link set up vcan0

cd build 	
cmake ..
make

# Terminal 1: Monitor
candump vcan0

# Terminal 2: Run
sudo ./car --can=vcan0
```

This program also includes a full test suite with 100% function coverage. If you wish to run the tests, follow these steps:
```shell
cd build
rm -rf *
cmake -DENABLE_COVERAGE=ON ..
make coverage
```

---

## Note
Root permissions are required due to CAN socket initialization and, when testing, to properly create a virtual interface that simulates a CAN transceiver inside the development machine.

## How To Expand the project
This program doesn't focus entirely on safety-critical aspects. However, there is a feature that must be as close to safety-critical as possible: the emergency brake. To ensure the message is sent as fast as possible, integration tests are necessary, and delay tests for the messages are also required.

Add an exclusive thread just for the emergency brake and explore struct sched_param to set the priority to the maximum level.

## Team members

- Jose Meneses
- Afonso Mota
