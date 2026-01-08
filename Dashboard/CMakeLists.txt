cmake_minimum_required(VERSION 3.16)
project(Car_1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(QT_HOST_PATH "~/Qt/6.7.3/gcc_64")
set(QT_HOST_PATH_CMAKE_DIR "~/Qt/6.7.3/gcc_64")
set(CMAKE_PREFIX_PATH "${QT_HOST_PATH_CMAKE_DIR}" CACHE PATH "Qt host path" FORCE)
# encontre o Qt ANTES de usar recursos
find_package(Qt6 REQUIRED COMPONENTS Svg Quick Qml Gui Core PATHS ${CMAKE_PREFIX_PATH})

add_executable(Car_1
    srcs/main.cpp
    srcs/radialbar.cpp
    incs/radialbar.h
    srcs/qml.qrc
)

target_include_directories(Car_1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/incs
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Car_1 PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Core
    Qt6::Svg
)
