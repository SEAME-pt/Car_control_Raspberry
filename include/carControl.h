#pragma once

//libs:
//standart/general use
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <memory>

//joystick
#include <SDL2/SDL.h>

//speedSensor
#include <thread>
#include <pigpio.h>
#include <atomic>

#include "../libs/include/I2c.hpp"
#include "exceptions.hpp"
#include "CANController.hpp"

//init
SDL_Joystick*	initCar();
void	        initI2c();

//utils
double			mapAxisToAngle(double axisValue, double angleMin, double angleMax, double angleCenter);

//exit
void	        exitCar();
void	        exitSDL();
void	        signalHandler(int signum);
void			cleanExit();

extern SDL_Joystick*    g_joystick;
extern bool				g_running;
extern std::unique_ptr<CANController>	g_can;

//index of the controller, if 0, the first, 
//and only the first, has permission to connect
#define CONTROLLER_0    0

//value of the axis in the controller
#define MAX_AXIS_VALUE	32767.0f

//Numbers of pulses from each rotation (how many holes the wheel has)
#define	PULSES_WHEEL	20

//angle to centralize the servo of the car and use it as the default angle
#define MID_ANGLE       60

#define A_BUTTON		0
#define B_BUTTON		1
#define X_BUTTON		3
#define Y_BUTTON		4
#define L1_BUTTON		6
#define R1_BUTTON		7
#define L2_BUTTON		8
#define R2_BUTTON		9
#define START_BUTTON	11
