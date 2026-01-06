# TEAM1 - Velocity Era

This repository consolidates the most up to date software stack for our autonomous vehicle, Vera, which runs on a Raspberry Pi 5. Currently, the program fully integrates Joystick support, capturing axis values and converting them into I²C data. These values are then transmitted via CAN-FD to the microcontroller, enabling safe and reliable control of Vera’s movement in a safety critical environment.

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
TEAM1-Car_control_Raspberry/
│
├── CMakeLists.txt                       # Build configuration
├── README.md                            # Project documentation
│
├── include/                             # Header files
│   ├── CANController.hpp                # CAN controller class (smart pointer)
│   ├── CANProtocol.hpp                  # CAN protocol definitions
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
│   ├── init/                            # Initialization modules
│   │   ├── init.cpp                     # Main initialization
│   │
│   └── utils/                           # Utility functions
│       └── parsing.cpp                  # Command-line argument parsing
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
```

CAN instructions and their respective id's are:

Raspberry Pi -> STM32:

```cpp
namespace CANID {
	constexpr uint32_t EMERGENCY_BRAKE = 0x100;		// max priority
	constexpr uint32_t DRIVE_COMMAND   = 0x101;		// high priority
	constexpr uint32_t DRIVE_MODE      = 0x102;		// low priority
};
```

STM32 -> Raspberry Pi

```cpp
namespace CANID {
	constexpr uint32_t MOTOR_SERVO		= 0x200;	// max priority
	constexpr uint32_t SPEED_SENSOR		= 0x201;	// high priority
	constexpr uint32_t ERROR_UPDATES	= 0x202;	// low priority
};
```

## Joystick

Initialization of SDL to read Joystick input:

```cpp
// Initialize SDL Joystick subsystem
SDL_Init(SDL_INIT_Joystick);

// Get number of connected Joysticks
SDL_NumJoysticks();

// Open first Joystick
SDL_Joystick *Joystick = SDL_JoystickOpen(CONTROLLER_0);
```

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
sudo ip link set vcan0 mtu 72
sudo ip link set up vcan0

cd build 	
cmake ..
make

# Terminal 1: Monitor
candump vcan0

# Terminal 2: Run
sudo ./car --can=vcan0 --debug
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

## Team members

- Jose Meneses
- Afonso Mota
