cmake_minimum_required(VERSION 3.16)
project(car)

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    srcs/main.cpp
	#utils
	srcs/utils/parsing.cpp
	#init
	srcs/init/init_can.cpp
	srcs/init/init_joystick.cpp
	srcs/init/init.cpp
	#controller
    srcs/controller/joystick.cpp
	#can
    srcs/can/socketCAN.c
    srcs/can/CANController.cpp
)

# Executable
add_executable(car ${SOURCES})

# Link libraries
target_link_libraries(car PRIVATE ${SDL2_LIBRARIES})

# Summary
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "SDL2 version: ${SDL2_VERSION}")
message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
message(STATUS "SDL2 include: ${SDL2_INCLUDE_DIRS}")
message(STATUS "================================")