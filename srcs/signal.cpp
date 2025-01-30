#include <csignal>
#include <iostream>
#include <sys/wait.h>

extern int g_signal;

static void sigQuitHandler(int sig) {
	std::cout << " Quit signal (" << sig << ") received.\n";
	g_signal = sig;
}

// Signal handler function to reap zombie processes 
void sigChldHandler(int signum) { 
	pid_t pid; 
	int status; 
	
	std::cout << " Child termination signal (" << signum << ") received.\n";
	// Reap all zombie processes 
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { 
		std::cerr << "Parent process reaped child process with PID " << pid << "." << std::endl; 
	}

	std::cout << " Child termination signal (" << signum << ") handled.\n";
}

int setupSignalHandlers(void) {
	if (SIG_ERR == signal(SIGQUIT, &sigQuitHandler)) {
		return (-1);
	}
	// if (SIG_ERR == signal(SIGCHLD, &sigChldHandler)) {
	// 	return (-1);
	// }
	// if (SIG_ERR == signal(SIGINT, &sigHandler)) {
	// 	return (-1);
	// }
	return (0);
}
