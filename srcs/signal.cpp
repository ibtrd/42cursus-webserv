#include <iostream>
#include <csignal>

extern int g_signal;

static void	sigHandler(int sig) {
	std::cout << " Interrupt signal (" << sig << ") received.\n";
	g_signal = sig;
}

int	setupSignalHandlers(void) {
	if (SIG_ERR == signal(SIGQUIT, &sigHandler)) {
		return (-1);
	}
	// if (SIG_ERR == signal(SIGINT, &sigHandler)) {
	// 	return (-1);
	// }
	return (0);
}
