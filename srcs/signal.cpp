#include "IRC.hpp"

void signalHandler(int signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		g_running = 0;
	}
}