cmake_minimum_required(VERSION 3.16)
project(TEAM1_Car_Control_CAN LANGUAGES C CXX)

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

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
	#can_fd
    srcs/can/socketCAN.c
    srcs/can/CANController.cpp
)

# Executable
add_executable(car ${SOURCES})

target_include_directories(car PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(car PRIVATE
    ${SDL2_LIBRARIES}
)

# Summary
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SDL2:       ${SDL2_LIBRARIES}")
message(STATUS "================================")