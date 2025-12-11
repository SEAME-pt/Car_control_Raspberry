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
    std::cout << "Opened: " << libevdev_get_name(dev) << " (bus="
              << libevdev_get_id_bustype(dev) << " vendor="
              << libevdev_get_id_vendor(dev) << " product="
              << libevdev_get_id_product(dev) << ")\n";
    if (libevdev_grab(dev, LIBEVDEV_GRAB) != 0) {
		throw std::runtime_error(std::string("joyStick: failed to grab device"));
	}
}

joyStick::~joyStick(){
    libevdev_free(dev);
    close(fd);
}

void	joyStick::getAbs(void) const{
    // Example: print ABS ranges if available
    for (int code = 0; code <= ABS_MAX; ++code) {
        if (libevdev_has_event_code(dev, EV_ABS, code)) {
            const struct input_absinfo *ai = libevdev_get_abs_info(dev, code);
            if (ai) {
                std::cout << "ABS " << libevdev_event_code_get_name(EV_ABS, code)
                          << " min=" << ai->minimum << " max=" << ai->maximum << "\n";
            }
        }
    }

}

__u16	joyStick::readPress(void) {
	rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
	if (rc == 0) {
		if (ev.type != EV_SYN) {
			// print type/code/value with human-readable names
			const char *tname = libevdev_event_type_get_name(ev.type);
			const char *cname = libevdev_event_code_get_name(ev.type, ev.code);
			if (!tname) tname = "TYPE?";
			if (!cname) cname = "CODE?";
			std::cout << "wtf " << ev.code << std::endl;
			return ev.code;
		}
	} else if (rc != -EAGAIN) {
		throw std::runtime_error(std::string("joyStick: device removed"));
	}
	return (0);
}
