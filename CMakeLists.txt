cmake_minimum_required(VERSION 3.16)
project(TEAM1_Car_Control_I2C LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

message(STATUS "=== TEAM1 Car Control I2C ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

# =============================================================================
# Find required packages
# =============================================================================

# Threads (required for std::thread)
find_package(Threads REQUIRED)

# SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
message(STATUS "SDL2 found: ${SDL2_LIBRARIES}")

# pigpio library
find_library(PIGPIO_LIB pigpio REQUIRED)
if(NOT PIGPIO_LIB)
    message(FATAL_ERROR "pigpio library not found. Install: sudo apt-get install pigpio")
endif()
message(STATUS "pigpio found: ${PIGPIO_LIB}")

# =============================================================================
# Add I2C library subproject (has its own CMakeLists.txt)
# =============================================================================
add_subdirectory(libs)

# =============================================================================
# Main executable
# =============================================================================

set(CAR_SOURCES
    srcs/main.cpp
    srcs/exit/exitCleanups.cpp
    srcs/exit/signals.cpp
    srcs/init/init_car_i2c.cpp
    srcs/utils/math_utils.cpp
)

add_executable(car ${CAR_SOURCES})

# Include directories
target_include_directories(car PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(car PRIVATE
    i2c_lib                 # From libs/CMakeLists.txt
    ${SDL2_LIBRARIES}
    ${PIGPIO_LIB}
    Threads::Threads
    rt                      # Real-time extensions (needed by pigpio)
)

# Add SDL2 compile flags
target_compile_options(car PRIVATE ${SDL2_CFLAGS_OTHER})

# =============================================================================
# Installation
# =============================================================================
install(TARGETS car
    RUNTIME DESTINATION bin
)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "=================================")
message(STATUS "Configuration complete!")
message(STATUS "  Build with: make -j$(nproc)")
message(STATUS "  Run with:   sudo ./car")
message(STATUS "=================================")
