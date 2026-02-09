#include "carControl.h"

// Temp function just to print receiving vaues for now
uint16_t	rpmToSpeedMps(uint16_t rpm) {
    
	double speed_mps = (rpm * WHEEL_CIRCUMFERENCE_M) / 60.0;
	std::cout << "Currently m/s is: " << static_cast<uint16_t>(speed_mps) << std::endl;
    return (static_cast<uint16_t>(speed_mps));
}
