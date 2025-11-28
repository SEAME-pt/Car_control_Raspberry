#include "../../include/carControl.h"

double	mapAxisToAngle(double axisValue, double angleMin, double angleMax, double angleCenter) {

	if (axisValue < 0) {
		// Map [-1, 0] -> [angleMin, angleCenter]
		return ((angleCenter - angleMin) * (axisValue - (-1)) / (0 - (-1)) + angleMin);
	} else {
		// Map [0, 1] -> [angleCenter, angleMax]
		return ((angleMax - angleCenter) * (axisValue - 0) / (1 - 0) + angleCenter);
	}
}
