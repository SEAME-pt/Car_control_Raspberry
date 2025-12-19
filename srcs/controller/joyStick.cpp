#include "carControl.h"
#include "joyStick.hpp"

joyStick::joyStick(const char *device)
{
	if (!device)
		throw std::runtime_error(std::string("joyStick: Device cant be Null"));
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
		throw std::runtime_error(std::string("joyStick: failed to open device"));
    }
    if (libevdev_new_from_fd(fd, &dev) < 0) {
        close(fd);
		throw std::runtime_error(std::string("joyStick: failed init libevdev for device"));
    }
    //std::cout << "Opened: " << libevdev_get_name(dev) << " (bus="
    //          << libevdev_get_id_bustype(dev) << " vendor="
    //          << libevdev_get_id_vendor(dev) << " product="
    //          << libevdev_get_id_product(dev) << ")\n";
}

joyStick::~joyStick(){
    libevdev_free(dev);
    close(fd);
}

int16_t	joyStick::getAbs(bool steering) const{
    for (int code = 0; code <= ABS_MAX; ++code) {
        if (libevdev_has_event_code(dev, EV_ABS, code)) {
            const struct input_absinfo *ai = libevdev_get_abs_info(dev, code);
            if (ai && steering && code == ABS_X) {
                return (ai->value - 127);
            } else if (ai && !steering && code == ABS_Y) {
                return ((ai->value - 127) * -1);
            }
        }
    }

}

int16_t	joyStick::readPress(void) {
	rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
	if (rc == 0) {
		if (ev.type != EV_SYN && ev.value != 0) { // Only consider key/button press events
            return (ev.code - 304);
		}
	} else if (rc != -EAGAIN) {
		throw std::runtime_error(std::string("joyStick: device removed"));
	}
	return (-1);
}
