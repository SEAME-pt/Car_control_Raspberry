#pragma once

#include <libevdev/libevdev.h>
#include <filesystem>

class Joystick {
	private:
		struct libevdev	*dev = nullptr;
		struct input_event ev;
		int rc;
		int	fd;
		std::string	_device;
	public:
		Joystick();
		~Joystick();
	
		int8_t		getAbs(bool steering) const;
		int8_t		readPress(void);
		void		findJoystickDevice();
};
