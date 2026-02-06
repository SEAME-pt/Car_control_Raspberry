#pragma once

#include "CANController.hpp"
#include "CANProtocol.hpp"
#include "Joystick.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <iomanip>
#include <queue>
#include <mutex>

/**
 * @file carControl.hpp
 * @brief High-level vehicle control abstractions for manual and autonomous operation.
 *
 * Provides initialization, main loops, signal handling, and CAN/Joystick integration.
 */

//index of the controller, if 0, the first, 
//and only the first, has permission to connect
#define CONTROLLER_0    0

//value of the axis in the controller
#define MAX_AXIS_VALUE	32767.0f

//angle to centralize the servo of the car and use it as the default angle
#define MID_ANGLE		60

#define A_BUTTON		0
#define B_BUTTON		1
#define X_BUTTON		3
#define Y_BUTTON		4
#define L1_BUTTON		6
#define R1_BUTTON		7
#define L2_BUTTON		8
#define R2_BUTTON		9
#define START_BUTTON	11

// Wheel values
#define WHEEL_CIRCUMFERENCE_M	0.21

/**
 * @struct s_carControl
 * @brief Aggregates all vehicle control objects and configuration.
 *
 * Contains the CAN controller, joystick, interface name, and operation mode.
 */
typedef struct s_carControl {

	std::unique_ptr<CANController>	can;
	std::unique_ptr<Joystick>		controller;
	std::string		canInterface;
	bool			manual;
	bool			exit;
} t_carControl;

/**
 * @struct s_speedData
 * @brief Speed sensor data
 */
typedef struct s_speedData {
	uint16_t	rpm;
	uint16_t	speedMps;
} t_speedData;

/**
 * @struct s_batteryData
 * @brief Battery sensor data
 */
typedef struct s_batteryData {
	uint16_t	voltage;
	uint8_t		percentage;
} t_batteryData;

/**
 * @struct s_heartbeatData
 * @brief Heartbeat acknowledgment from STM32
 */
typedef struct s_heartbeatData {
	uint8_t	ack;
	std::chrono::steady_clock::time_point timestamp;
} t_heartbeatData;

/**
 * @struct s_CANReceiver
 * @brief Central CAN message receiver with thread-safe queues
 */
typedef struct s_CANReceiver {
	std::queue<t_speedData>		speedQueue;
	std::queue<t_batteryData>	batteryQueue;
	std::queue<t_heartbeatData>	heartbeatQueue;

	std::mutex speedMutex;
	std::mutex batteryMutex;
	std::mutex heartbeatMutex;

	CANController*	can;
} t_CANReceiver;

/**
 * @brief Initialize a CAN controller instance.
 *
 * @param interface CAN interface name (e.g., "can0")
 * @return Unique pointer to a CANController
 * @throws CANController::CANException if initialization fails
 */
std::unique_ptr<CANController>	
				init_can(const std::string &interface);

/**
 * @brief Initializes core car control components.
 *
 * Sets up joystick, CAN controller, and parses command line arguments.
 *
 * @param argc Argument count from main
 * @param argv Argument values from main
 * @return Initialized t_carControl struct
 */
t_carControl	initCarControl(int argc, char *argv[]);

/**
 * @brief Parse command line arguments and update carControl structure.
 *
 * Supported options:
 * - --manual=true|false
 * - --can=INTERFACE
 * - --help or -h
 *
 * @param argc Argument count
 * @param argv Argument values
 * @param carControl Pointer to t_carControl to update
 * @return 1 if parsing successful, 0 if exit requested
 */
int	parsingArgv(int argc, char *argv[],
		t_carControl *carControl);

/**
 * @brief Main loop for manual joystick control.
 *
 * Aggregates joystick outputs, stabilizes values, and sends CAN frames.
 *
 * @param carControl Pointer to t_carControl containing CAN and joystick
 */
void	manualLoop(t_carControl *carControl, t_CANReceiver* receiver);

/**
 * @brief Main loop for autonomous operation.
 *
 * Sends emergency brake commands periodically and reads CAN bus messages.
 *
 * @param carControl Reference to t_carControl
 */
void	autonomousLoop(const t_carControl &carControl);

/**
 * @brief Sets up signal handling for graceful shutdown (SIGINT, SIGTERM)
 */
void	signalManager();

/**
 * @brief Signal handler to stop the main loops safely.
 *
 * @param signum Signal number
 */
void	signalHandler(int signum);

/**
 * @brief Converts raw RPM into meters per second
 *
 * @param can Reference raw RPM message received from stm32
 */
uint16_t	rpmToSpeedMps(uint16_t rpm);

/**
 * @brief CAN receiver thread - reads all CAN messages and distributes to queues
 * 
 * @param receiver Pointer to CANReceiver structure
 */
void	canReceiverThread(t_CANReceiver* receiver);

void	heartbeatThread(t_CANReceiver* receiver);

/**
 * @brief Get latest speed data from queue (non-blocking)
 * 
 * @param receiver Pointer to CANReceiver
 * @param data Output speed data
 * @return true if data available, false if queue empty
 */
bool	getSpeedData(t_CANReceiver* receiver, t_speedData* data);

/**
 * @brief Get latest battery data from queue (non-blocking)
 * 
 * @param receiver Pointer to CANReceiver
 * @param data Output battery data
 * @return true if data available, false if queue empty
 */
bool	getBatteryData(t_CANReceiver* receiver, t_batteryData* data);

/**
 * @brief Get latest heartbeat ACK from queue (non-blocking)
 * 
 * @param receiver Pointer to CANReceiver
 * @param data Output heartbeat data
 * @return true if data available, false if queue empty
 */
bool	getHeartbeatAck(t_CANReceiver* receiver, t_heartbeatData* data);

/**
 * @brief Global atomic flag controlling main loops.
 *
 * Set to false to terminate manualLoop or autonomousLoop safely.
 */
extern std::atomic<bool> g_running;
