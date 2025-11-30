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

# Find pigpio (optional for I2C)
find_library(PIGPIO_LIB pigpio)
if(PIGPIO_LIB)
    set(HAVE_I2C ON)
    add_subdirectory(libs)
else()
    message(WARNING "pigpio not found. I2C disabled.")
    set(HAVE_I2C OFF)
endif()

# Source files
set(SOURCES
    srcs/main.cpp
    srcs/can_fd/socketCAN.c
    srcs/can_fd/CANController.cpp
    srcs/controller/joystick.cpp
    srcs/init/init_can.cpp
)

# Add I2C init if available
if(HAVE_I2C)
    list(APPEND SOURCES srcs/init/init_car_i2c.cpp)
endif()

# Executable
add_executable(car ${SOURCES})

target_include_directories(car PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(car PRIVATE
    ${SDL2_LIBRARIES}
)

if(HAVE_I2C)
    target_link_libraries(car PRIVATE
        i2c_lib
        ${PIGPIO_LIB}
    )
    target_compile_definitions(car PRIVATE HAVE_I2C)
endif()

# Summary
message(STATUS "================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SDL2:       ${SDL2_LIBRARIES}")
message(STATUS "I2C:        ${HAVE_I2C}")
message(STATUS "================================")