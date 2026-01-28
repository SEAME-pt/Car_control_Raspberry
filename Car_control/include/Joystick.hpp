#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <libevdev/libevdev.h>
#include <filesystem>

/**
 * @file Joystick.hpp
 * @brief Interface for reading joystick input via libevdev.
 *
 * Provides functions for accessing axes and buttons,
 * handling deadzones, and stabilizing input values for embedded control.
 */

class Joystick {
	private:
		struct libevdev		*dev = nullptr;	/**< libevdev device handle */
		struct input_event	ev;				/**< Last input event */
		int 				rc;				/**< Return code for libevdev calls */
		int					fd;				/**< File descriptor for joystick device */
		std::string			_device;		/**< Device path (e.g., /dev/input/eventX) */
	public:
		/**
		 * @brief Constructor - finds and initializes the joystick device.
		 *
		 * Throws std::runtime_error if no device is found or initialization fails.
		 */
		Joystick();
		/**
		 * @brief Destructor - frees libevdev and closes file descriptor.
		 */
		~Joystick();
	
		/**
		 * @brief Reads an absolute axis value from the joystick.
		 *
		 * @param steering If true, reads steering (X axis). If false, reads throttle (Y axis).
		 * @return Clamped axis value (0–120 for steering, -100–100 for throttle), or -1 on error
		 */
		int16_t	getAbs(bool steering) const;

		/**
		 * @brief Reads button press events.
		 *
		 * Only considers key/button press events (ignores EV_SYN and releases).
		 *
		 * @return Button code (ev.code - 304) if pressed, -1 if no button press
		 * @throws std::runtime_error if device is removed or read fails
		 */
		int		readPress(void);

		/**
		 * @brief Detects connected joystick device and sets _device path.
		 *
		 * @throws std::runtime_error if no joystick is found
		 */
		void	findJoystickDevice();
};

/**
 * @brief Stabilizes joystick input values to handle deadzones and calibration.
 *
 * @param steering Pointer to steering value to stabilize
 * @param throttle Pointer to throttle value to stabilize
 */
void	stableValues(int16_t *steering, int16_t *throttle);
