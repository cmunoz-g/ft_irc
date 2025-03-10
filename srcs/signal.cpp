#include "IRC.hpp"

void signalHandler(int signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		std::cout << "Hello!" << std::endl;
		g_running = 0;
	}
}