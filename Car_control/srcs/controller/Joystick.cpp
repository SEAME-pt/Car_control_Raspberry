#include "Joystick.hpp"

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

// Returns clamped axis value for steering & throttle || -1 on error
int16_t	Joystick::getAbs(bool steering) const {

	for (int code = 0; code <= ABS_MAX; ++code) {
		if (libevdev_has_event_code(dev, EV_ABS, code)) {
			const struct input_absinfo *ai = libevdev_get_abs_info(dev, code);
			if (ai && steering && code == ABS_X) {
				int range = ai->maximum - ai->minimum;
                int normalized = ((ai->value - ai->minimum) * 120) / range;
                return static_cast<int16_t>(std::clamp(normalized, 0, 120));
			}

			else if (ai && !steering && code == ABS_Y) {
				int range = ai->maximum - ai->minimum;
                int center = (ai->maximum + ai->minimum) / 2;
                int normalized = ((center - ai->value) * 200) / range;
                return static_cast<int16_t>(std::clamp(normalized, -100, 100));
			}
		}
	}
	return (-1);
}

int	Joystick::readPress(void) {
	rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
	if (rc == 0) {
		if (ev.type != EV_SYN && ev.value != 0) // Only consider key/button press events
			return (ev.code - 304);

	} else if (rc != -EAGAIN)
		throw std::runtime_error(std::string("Error! Joystick device removed, initiating forcing shutdown..."));

	return (-1);
}

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
