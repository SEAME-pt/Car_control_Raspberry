#include "carControl.h"

// Steering position from joystick
int8_t	joystickSteering(SDL_Joystick* joystick) {

	constexpr float	MAX_STEERING = 120.0f;
	constexpr float	MIN_STEERING = 0.0f;
	constexpr float	STEERING_CENTER = 60.0f;
	constexpr int	STEERING_AXIS = 2;

	float axisSteering = SDL_JoystickGetAxis(joystick, STEERING_AXIS) / MAX_AXIS_VALUE;

	uint8_t steering = static_cast<uint8_t>(
		std::clamp(axisSteering * STEERING_CENTER + STEERING_CENTER, MIN_STEERING, MAX_STEERING)
	);
	return (steering);
}

// Throttle position from joystick
int8_t	joystickThrottle(SDL_Joystick* joystick) {
	
	constexpr float	MAX_THROTTLE = 100.0f;
	constexpr float	MIN_THROTTLE = -100.0f;
	constexpr float	THROTTLE_SCALE = 100.0f;
	constexpr int	THROTTLE_AXIS = 1;

	float	axisThrottle = SDL_JoystickGetAxis(joystick, THROTTLE_AXIS) / MAX_AXIS_VALUE;
		
	int8_t throttle = static_cast<int8_t>(
		std::clamp(-axisThrottle * THROTTLE_SCALE, MIN_THROTTLE, MAX_THROTTLE)
	);
	return (throttle);
}
