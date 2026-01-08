#include "carControl.h"

void	stableValues(int16_t *steering, int16_t *throttle) {

	const int16_t STEERING_CENTER = 60;
	const int16_t STEERING_DEADZONE = 2;
	const int16_t THROTTLE_DEADZONE = 2;

	if (*steering > STEERING_CENTER - STEERING_DEADZONE && 
        *steering < STEERING_CENTER + STEERING_DEADZONE)
        *steering = STEERING_CENTER;
    
    if (*throttle > -THROTTLE_DEADZONE && *throttle < THROTTLE_DEADZONE)
        *throttle = 0;
}
