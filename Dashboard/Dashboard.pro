TEMPLATE = app
QT += quick network
SOURCES += srcs/main.cpp srcs/Dashboard.cpp srcs/CarDataController.cpp
HEADERS += incs/Dashboard.hpp incs/CarDataController.hpp
RESOURCES += qml.qrc

# Add additional QML files if needed
# QML_FILES += srcs/Main.qml

# Set the target name
TARGET = Dashboard
