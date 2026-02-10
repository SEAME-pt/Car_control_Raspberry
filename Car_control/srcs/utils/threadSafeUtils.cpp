#include "carControl.h"

// Thread-safe helper functions
bool	getSpeedData(t_CANReceiver* receiver, t_speedData* data, bool pop) {

	std::lock_guard<std::mutex> lock(receiver->speedMutex);
	if (!receiver->speedQueue.empty()) {

		*data = receiver->speedQueue.front();
		if (pop) {
			receiver->speedQueue.pop();
		}
		return (true);
	}
	return (false);
}

bool	getBatteryData(t_CANReceiver* receiver, t_batteryData* data, bool pop) {

	std::lock_guard<std::mutex> lock(receiver->batteryMutex);
	if (!receiver->batteryQueue.empty()) {

		*data = receiver->batteryQueue.front();
		if (pop) {
			receiver->batteryQueue.pop();
		}
		return (true);
	}
	return (false);
}
