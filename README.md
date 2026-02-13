# TEAM1 - Velocity Era

This repository consolidates the most up to date software stack for our autonomous vehicle, Vera and, which runs on a Raspberry Pi 5. Currently, the program fully integrates Joystick support, capturing axis values and converting them to messages. These values are then transmitted via CAN-FD to the microcontroller, enabling safe and reliable control of Vera’s movement in a safety critical environment.

Additionally, the program is responsible for running the Qt interface, which displays on the car’s screen the data received from the microcontroller (such as speed sensor readings and motor/servo feedback) also communicated through CAN.

## Overview

Components of this project:
- Raspberry Pi 5
- CAN-BUS(FD) HAT for Raspberry Pi
- Joystick

This platform integrates dependencies like:
- **Car_control**: LIBEVDEV, SocketCAN, Google Tests, gcov, lcov, doxygen
- **Dashboard**: Qt6/Qt5, QML, Qt Quick Controls, Google Tests

---

## File Structure


### Car Control
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
│   │   ├── manual_mode.cpp 			 # Manual joystick control loop
│   │   └── monitoring_thread.cpp 		 # STM32 health monitoring thread
│   │
│   ├── init/                            # Initialization modules
│   │   ├── init.cpp                     # Main initialization
│   │   └── init_can.cpp 		 		 # CAN initialization helper
│   │
│   └── utils/                           # Utility functions
│   |   ├── inputParsing.cpp 				 # Command-line argument parsing
│   |   ├── signal.cpp 					 # Signal handling (graceful shutdown)
│   |   ├── canRxParsing.cpp 		 # CAN receive debug utility
│   │   └── threadSafeUtils_thread.cpp 		 # STM32 health monitoring thread
│
└── build/                               # Build output (gitignored)
    ├── Makefile
    └── car                              # Executable
```

### Dashboard

```
Dashboard/
│
├── Dashboard.pro                        # Qt project file
├── qml.qrc                              # QML resource file
│
├── incs/                                # Header files
│   ├── CarDataController.hpp            # Car data controller class
│   ├── CoverArtProvider.hpp             # Cover art provider for music
│   ├── Dashboard.hpp                    # Main dashboard class
│   └── DashboardMacros.hpp              # Dashboard macros and constants
│
├── srcs/                                # Source files
│   ├── main.cpp                         # Main entry point
│   ├── Dashboard.cpp                    # Main dashboard implementation
│   ├── CarDataController.cpp            # Car data controller implementation
│   ├── CoverArtProvider.cpp             # Cover art provider implementation
│   │
│   ├── BotRow/                          # Bottom row UI components
│   ├── CenterDisplay/                   # Center display UI components
│   ├── ErrorDisplay/                    # Error display UI components
│   ├── LeftDisplay/                     # Left display UI components
│   ├── RightDisplay/                    # Right display UI components
│   └── TopRow/                          # Top row UI components
│
├── assets/                              # UI assets (images, etc.)
├── fonts/                               # Custom fonts
├── docs/                                # Documentation
├── tests/                               # Test files
└── build/                               # Build output (gitignored)
    ├── Makefile
    └── dashoard                         # Executable

```

```
├── scripts/                                 # Installation and utility scripts
│   ├── install_doxygen.sh
│   ├── install_GoogleTest.sh
│   ├── install_lcov.sh
│   ├── install_libevdev.sh
│   └── send_test_data.py                    # Helper script to send simulated data to Dashboard
│
├── CI_CD.yml                                # CI/CD configuration
├── README.md                                # This file
├── socketCAN.md                             # SocketCAN documentation
└── Doxyfile                                 # Doxygen configuration
```

### 1. Install dependencies

#### Car_control Dependencies
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
```

#### Dashboard Dependencies
```shell
# Qt6 (recommended)
sudo apt-get install qt6-base-dev qt6-declarative-dev qt6-quickcontrols2-dev

# Google Test (for testing, already installed by scripts above)
```

# Compile For Raspberri Pi 5 (AGL)


### Car control
```shell
cd build
cmake .. -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DBUILD_TESTS=OFF
make
scp car root@<RASPBERRY_IP>:/root/
```

### Dashboard

```bash
cd Dashboard
mkdir -p build
cd build

# Source your AGL SDK environment (adjust path to your SDK installation)
source /opt/agl-sdk/20.90.0-aarch64/environment-setup-aarch64-agl-linux

qmake6 ../Dashboard.pro CONFIG+=release
make
scp dashboard root@<RASPBERRY_IP>:/root/
```

**Note**: Adjust the `source` path to match your AGL SDK installation location and version.

Is this your first time running the program? If so, check out how to properly install all dependencies:

# Run the program


## Car Conrol

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

## Dashboard

```shell
cd build
qmake6 .. && make
./dashboard
```

# Testing & Coverage

## Car Control

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

## Dashboard

```bash
cd Dashboard/tests
mkdir -p build
cd build

# Build tests
cmake .. -DENABLE_COVERAGE=ON
cmake --build .

# Run tests
ctest --output-on-failure --verbose

# Generate coverage report
cmake --build . --target coverage
# Coverage report will be in: coverage_html/index.html
```---

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

## Dashboard Documentation

The Dashboard includes additional documentation:

- **SOCKET_DATA.md**: Socket communication protocols and data formats
- **ALBUM_ART.md**: Cover art handling and image processing

Located in `Dashboard/docs/`

# Note
Root permissions are required due to CAN socket initialization and, when testing, to properly create a virtual interface that simulates a CAN transceiver inside the development machine.

## How To Expand the project
This program focus on safety-critical aspects. Having that in mind, there is a special feature that must be closer to a safety-critical manner: the emergency brake. To ensure the message is sent as fast as possible, integration tests are necessary, and delay tests for the messages are also required.

Add an exclusive thread just for the emergency brake and explore struct sched_param to set the priority to the maximum level.

## Team members

- Jose Meneses
- Afonso Mota
