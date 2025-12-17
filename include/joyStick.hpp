
#pragma once

#include <libevdev/libevdev.h>

class joyStick {
	private:
		struct libevdev	*dev = nullptr;
		struct input_event ev;
		int rc;
		int	fd;
	public:
		joyStick(const char *device);
		~joyStick();
	
		void	getAbs(void) const;
		int16_t	readPress(void);
};
