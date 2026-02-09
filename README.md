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
- gcov
- lcov
- doxygen

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

---

# Build & Run

### 1. Install dependencies

## Intructions to compile and flash
In order to compile the program and flash the binary into the raspberry Pi 5 (AGL), follow the steps:

```shell
cd build
cmake .. -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_TESTS=OFF
make
scp car root@<RaspPi5>:/root/
```

## Instructions to run the program

Is this your first time running the program? If so, check out how to properly install all dependencies:

```shell
cd scripts
chmod +x install_lcov.sh
chmod +x install_GoogleTest.sh
chmod +x install_libevdev.sh
chmod +x install_doxygen.sh

./install_lcov.sh
./install_GoogleTest.sh
./install_libevdev.sh
./install_doxygen.sh

cd .. && mkdir build && cd build
cmake ..
make
```

# Compile For Raspberri Pi 5 (AGL)

```shell
cd build
cmake .. -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_TESTS=OFF
make
scp car root@<RASPBERRY_IP>:/root/
```

# Run the program

```shell
cd build
cmake .. && make
sudo ./car --help 	# Shows all options and default values
sudo ./car          # Run with default values
```

### Debugging with virtual CAN

```shell
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 mtu 16   # or 72 if CAN-FD
sudo ip link set up vcan0

# Terminal 1: monitor
candump vcan0

# Terminal 2: run program
sudo ./car --can=vcan0
```

# Testing & Coverage

```shell
cd build
rm -rf *
# To build tests and run without joystick/hardware tests (recommended for CI/coverage):
cmake -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON ..
cmake --build . --target tests

# To run tests with joystick/hardware support enabled (requires enabling joystick at build time):
cmake -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON -DENABLE_JOYSTICK=ON ..
cmake --build . --target tests

# Run tests (executable created by the test build):
ctest --output-on-failure --verbose

# Generate coverage report (coverage target will skip hardware tests by default; pass -DSKIP_HARDWARE_TESTS=OFF to include them):
cmake -DENABLE_COVERAGE=ON -DSKIP_HARDWARE_TESTS=ON ..
cmake --build . --target coverage
```

---

# Documentation (Doxygen)

### Generate Doxygen configuration file (only if not yet generated):

```shell
doxygen -g
```

### Generate the docs:

```shell
doxygen Doxyfile
```

### Open the HTML documentation:

```shell
xdg-open docs/html/index.html
```

# Note
Root permissions are required due to CAN socket initialization and, when testing, to properly create a virtual interface that simulates a CAN transceiver inside the development machine.

## How To Expand the project
This program focus on safety-critical aspects. Having that in mind, there is a special feature that must be closer to a safety-critical manner: the emergency brake. To ensure the message is sent as fast as possible, integration tests are necessary, and delay tests for the messages are also required.

Add an exclusive thread just for the emergency brake and explore struct sched_param to set the priority to the maximum level.

## Team members

- Jose Meneses
- Afonso Mota
