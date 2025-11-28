#include "../../include/carControl.h"

void	signalHandler(int signum) {

    std::cout << "\nInterrupt operation (" << signum << ") received." << std::endl;
	g_running = false;
	cleanExit();
}
