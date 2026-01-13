cmake_minimum_required(VERSION 3.16)
project(uprotocol_speed_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automoc so Qt's moc runs automatically for Q_OBJECT classes in .cpp files
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# sensor daemon (no Qt)
add_executable(sensor_daemon sensor_daemon.cpp ../srcs/uprotocol.h)
target_compile_definitions(sensor_daemon PRIVATE _POSIX_C_SOURCE=200809L)