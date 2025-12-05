# TEAM1 - Velocity Era

Repository created to consolidate the most up to date software stack for our autonomous vehicle, affectionately called Vera that runs in a Raspberry Pi 5.

## Overview

This program requires a controller to capture joystick input, detect axis movement, and compute the commands that determine the car’s speed and steering direction. Its main purpose is to process these inputs and transmit the corresponding instructions to the microcontroller via CAN-FD, enabling the car to move accordingly.

Additionally, the program is responsible for running the Qt interface, which displays on the car’s screen the data received from the microcontroller (such as speed sensor readings and motor/servo feedback) also communicated through CAN-FD.

Components of this project:
- Raspberry Pi 5
- Joystick

This platform integrates tools like:
- SDL
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
│   │   └── joystick.cpp                 # SDL2 joystick handling
│   │
│   ├── init/                            # Initialization modules
│   │   ├── init.cpp                     # Main initialization
│   │   ├── init_can.cpp                 # CAN initialization (returns unique_ptr)
│   │   └── init_joystick.cpp             # Joystick
│   │
│   └── utils/                           # Utility functions
│       └── parsing.cpp                  # Command-line argument parsing
│
└── build/                               # Build output (gitignored)
    ├── CMakeCache.txt
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

Initialization of SDL to read joystick input:

```cpp
// Initialize SDL joystick subsystem
SDL_Init(SDL_INIT_JOYSTICK);

// Get number of connected joysticks
SDL_NumJoysticks();

// Open first joystick
SDL_Joystick *joystick = SDL_JoystickOpen(CONTROLLER_0);
```

# Build System

## Instructions to run the program

```bash
# Default values
# Notice that default values are prepared only for the final result of communication
# with STM32 and properly connection with joystick
cd build
cmake ..
make
sudo ./car

# For additional input information:
./car --help

# Testing purposes inside coding machine
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
sudo ./car --can=vcan0 --joy=false --debug

# Note: This program includes EXTENSIVE tests. To run them, follow the steps below:
cd build
rm -rf *
cmake -DENABLE_COVERAGE=ON ..
make coverage
```

# 3D Car Design

![car](https://github.com/user-attachments/assets/4c4beb0f-d26a-477a-bbbb-80e5452a20ef)

---

## Team members

- Jose Meneses
- Afonso Mota
- Rafael
- Joao Figueiredo
