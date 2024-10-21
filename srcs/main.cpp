#include <iostream>
#include <exception>

#include <Webserv.hpp>

static int	argvCheck(int argc, char *argv[]);

int main(int argc, char *argv[]) {

	if (argvCheck(argc, argv)) {
		return (1);
	}

	Webserv	server;

	try {
		server = Webserv(8080);
	} catch (std::exception &e) {
		std::cerr << "Fatal: " << e.what() << std::endl;
		return (1);
	}

	while (1) {
		server.recieveMessage();
	}
	return 0;
}

static int	argvCheck(int argc, char *argv[]) {
	bool	error = false;

	if (argc < 2) {
		std::cerr << "Error: missing argument" << std::endl;
		error = true;
	} else if (argc > 2) {
		std::cerr << "Error: too many arguments" << std::endl;
		error = true;
	}
	if (error) {
		std::cerr << "Usage: " << *argv << " [CONFIGURATION FILE]" << std::endl;
	}
	return (error);
}
