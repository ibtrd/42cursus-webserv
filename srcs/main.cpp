#include <iostream>

#include "Server.hpp"
#include "webserv.hpp"

int g_signal = 0;

int main(int argc, char *argv[]) {
	if (0 > setupSignalHandlers()) {
		return (1);
	}

	Server server;

	try {
		Configuration conf(argc, argv);

		if (conf.noRun()) {
			return 0;
		}
		server.configure(conf);
	} catch (std::invalid_argument &e) {
		std::cerr << "Error: " << e.what() << "\n";
		std::cerr << "Usage: " << *argv << " [CONFIGURATION FILE]" << std::endl;
		return (1);
	} catch (Configuration::ConfigurationException &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	} catch (std::exception &e) {
		std::cerr << "Fatal: " << e.what() << std::endl;
		return (1);
	}
	while (g_signal == 0) {
		server.routine();
	}
	return 0;
}
