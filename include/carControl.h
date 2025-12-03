#pragma once

#include "exceptions.hpp"
#include "CANController.hpp"
#include "CANProtocol.hpp"

// Signal
#include <csignal>

// Joystick
#include <SDL2/SDL.h>

typedef struct s_carControl {

	std::unique_ptr<CANController>	
					can;
	SDL_Joystick	*joystick;
	std::string		canInterface;
	bool			useJoystick;
	bool			debug;
	bool			exit;
} t_carControl;

//controller
int8_t			joystickSteering(SDL_Joystick* joystick);
int8_t			joystickThrottle(SDL_Joystick* joystick);

//init
std::unique_ptr<CANController>	
				init_can(const std::string &interface);
t_carControl	initCarControl(int argc, char *argv[]);
SDL_Joystick*	initJoystick();
void			cleanExit(SDL_Joystick* joystick);

//utils
int				parsingArgv(int argc, char *argv[], 
	t_carControl *carControl);

extern bool		g_running;

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
