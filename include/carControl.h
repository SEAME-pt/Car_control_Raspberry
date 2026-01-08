#pragma once

#include "exceptions.hpp"
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

typedef struct s_carControl {

	std::unique_ptr<CANController>	can;
	std::unique_ptr<Joystick>		controller;
	std::string		canInterface;
	bool			useJoystick;
	bool			debug;
	bool			exit;
} t_carControl;

//init
std::unique_ptr<CANController>	
				init_can(const std::string &interface);
t_carControl	initCarControl(int argc, char *argv[]);

//utils
int				parsingArgv(int argc, char *argv[],
				                    t_carControl *carControl);
void			signalManager();

extern	std::atomic<bool> g_running;
