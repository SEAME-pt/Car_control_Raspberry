#include "Joystick.hpp"

extern std::atomic<bool> g_running;

Joystick::Joystick() {

	findJoystickDevice();

	if (_device.empty())
		throw std::runtime_error(std::string("Error! Joystick device cant be Null..."));

	fd = open(_device.c_str(), O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		throw std::runtime_error(std::string("Error! Joystick device failed to open."));

	if (libevdev_new_from_fd(fd, &dev) < 0) {
		close(fd);
		throw std::runtime_error(std::string("Error! Failed libevdev init..."));
	}
}

Joystick::~Joystick() {
	libevdev_free(dev);
	close(fd);
}

/**
 * @brief Normalizes axis to appropriate range based on axis code.
 *
 * ABS_Z (steering): 0–180
 * ABS_Y (throttle): -100–100
 * Other axes: -100–100
 * Returns -1 if the axis is unavailable.
 */
int16_t	Joystick::getAbs(int axis_code) const {

	for (int code = 0; code <= ABS_MAX; ++code) {
		if (libevdev_has_event_code(dev, EV_ABS, code) && code == axis_code) {
			const struct input_absinfo *ai = libevdev_get_abs_info(dev, code);
			if (ai) {
				int range = ai->maximum - ai->minimum;
				if (axis_code == ABS_Z) { // Steering
					int normalized = ((ai->value - ai->minimum) * 180) / range;
					return static_cast<int16_t>(std::clamp(normalized, 0, 180));
				} else {
					int center = (ai->maximum + ai->minimum) / 2;
					int normalized = ((center - ai->value) * 200) / range;
					return static_cast<int16_t>(std::clamp(normalized, -100, 100));
				}
			}
		}
	}
	return (-1);
}

// Reads joystick buttons events pressed
int	Joystick::readPress(void) {
	struct input_event current_ev;

	rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &current_ev);
	if (rc == 0) {
		// Detect disconnect pattern: code 0 or 2 with value 128
		if (ev.code == 0 && current_ev.code == 2
			&& ev.value == 127 && current_ev.value == 127) {
			return (-2);
		} else if (current_ev.type != EV_SYN && current_ev.value != 0) { // Only consider key/button press events
			ev = current_ev; // Update last event
			return (ev.code - 304);
		}
	}
	return (-1);
}

// Check if controller is disconnected
bool	Joystick::isDisconnected(void) const {
	return disconnected;
}

// Find if device is connected and what's the name
void	Joystick::findJoystickDevice() {

	_device = "";
	const std::string inputPath = "/dev/input/by-id/";
	try {
		for (const auto& entry : std::filesystem::directory_iterator(inputPath)) {
			std::string filename = entry.path().filename().string();
			if (filename.find("-event-joystick") != std::string::npos)
				_device = entry.path().string();
		}
		return ;
	} catch (const std::filesystem::filesystem_error& e) {
		throw std::runtime_error("Error! No joystick device found.");
	}
}

// Function to stabilize input values.
// This addresses potential joystick calibration issues,
// ensuring that standard values remain reliable.
void stableValues(int16_t *steering, int16_t *throttle) {

    const int16_t STEERING_CENTER   = 60;
    const int16_t STEERING_DEADZONE = 2;
    const int16_t THROTTLE_DEADZONE = 2;
    const int16_t THROTTLE_MAX = 99;

    if (*steering > STEERING_CENTER - STEERING_DEADZONE && 
        *steering < STEERING_CENTER + STEERING_DEADZONE)
        *steering = STEERING_CENTER;
    
    if (*throttle > -THROTTLE_DEADZONE && *throttle < THROTTLE_DEADZONE)
        *throttle = 0;
    if (*throttle == THROTTLE_MAX)
        *throttle = 100;

    // Rounds throttle to a multiple of 10 
    *throttle = static_cast<int16_t>(std::round(*throttle / 10.0) * 10);

    // Rounds steering to a multiple of 5
    *steering = static_cast<int16_t>(std::round(*steering / 2.0) * 2);
}
