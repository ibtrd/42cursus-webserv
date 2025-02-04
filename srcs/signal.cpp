#include <sys/wait.h>

#include <iostream>

extern int g_signal;

static void sigQuitHandler(int sig) {
	std::cout << " Quit signal (" << sig << ") received.\n";
	g_signal = sig;
}

int setupSignalHandlers(void) {
	if (SIG_ERR == signal(SIGQUIT, &sigQuitHandler)) {
		return (-1);
	}
	return (0);
}
