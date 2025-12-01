#include "../../include/carControl.h"

// Steering position from joystick
int8_t	joystickSteering(SDL_Joystick* joystick) {

	float axisSteering = SDL_JoystickGetAxis(joystick, 2) / MAX_AXIS_VALUE;

	uint8_t steering = static_cast<uint8_t>(
        std::clamp(axisSteering * 60.0f + 60.0f, 0.0f, 120.0f)
    );
	return (steering);
}

// Throttle position from joystick
int8_t	joystickThrottle(SDL_Joystick* joystick) {
	
	float axisThrottle = SDL_JoystickGetAxis(joystick, 1) / MAX_AXIS_VALUE;
		
    int8_t throttle = static_cast<int8_t>(
        std::clamp(-axisThrottle * 100.0f, -100.0f, 100.0f)
    );
	return (throttle);
}
