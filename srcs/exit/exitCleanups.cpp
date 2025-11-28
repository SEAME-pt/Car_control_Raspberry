#include "../../include/carControl.h"

void	cleanExit() {

	if (g_joystick) {
		SDL_JoystickClose(g_joystick);
		g_joystick = nullptr;
	}

	I2c::All_close();
    SDL_Quit();
	exit(EXIT_SUCCESS);
}

void	exitSDL() {

	if (g_joystick) {
		SDL_JoystickClose(g_joystick);
		g_joystick = nullptr;
	}
	exit(EXIT_FAILURE);
}

void	exitCar() {

	if (g_joystick) {
		SDL_JoystickClose(g_joystick);
		g_joystick = nullptr;
	}
	I2c::All_close();
    SDL_Quit();
	exit(EXIT_FAILURE);
}

void	cleanup_can() {

    if (g_can) {
        std::cout << "🧹 Cleaning up CAN..." << std::endl;
        g_can.reset();
    }
}