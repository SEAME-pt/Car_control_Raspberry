#include "carControl.h"

// Condition for the main loop to keep running
volatile bool	g_running = true;

static void	signalHandler(int signum) {

	(void)signum;
	std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
}

int main(int argc, char *argv[]) {

	static int16_t last_steering = 0;
	static int16_t last_throttle = 0;

	t_carControl carControl = initCarControl(argc, argv);
	if (carControl.exit)
		return (1);

	try {

		CANProtocol::sendDriveMode(*carControl.can, DriveMode::MANUAL);
		CANProtocol::sendEmergencyBrake(*carControl.can, false);
		CANProtocol::sendDriveCommand(*carControl.can, MID_ANGLE, 0);

		signal(SIGINT, signalHandler);

		std::cout << "Starting Joystick reading loop..." << std::endl;
		while (g_running && !carControl.exit && carControl.controller != nullptr) {

			int		value = carControl.controller->readPress();
			int16_t	steering = carControl.controller->getAbs(true);
			int16_t	throttle = carControl.controller->getAbs(false);

			if (value == START_BUTTON) {
				std::cout << "Initiating graceful shutdown.." << std::endl;
				g_running = false;
			}

			if (steering == last_steering && throttle == last_throttle)
                continue;

			last_steering = steering;
            last_throttle = throttle;

			std::cout << "Steering: " << steering << " | Throttle: " << throttle << std::endl;
			int16_t steering_cmd = static_cast<int16_t>(((steering + 127) * 120) / 254);
            int16_t throttle_cmd = static_cast<int16_t>((throttle * 100) / 127);

			CANProtocol::sendDriveCommand(*carControl.can, steering_cmd, throttle_cmd);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
	CANProtocol::sendEmergencyBrake(*carControl.can, true);
	return (0);
}
