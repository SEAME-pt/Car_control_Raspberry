#include "carControl.h"

std::atomic<bool> g_running = true;

void	signalHandler(int signum) {
	(void)signum;
	g_running.store(false);
}

void	signalManager() {

	struct sigaction sa{};
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
}
