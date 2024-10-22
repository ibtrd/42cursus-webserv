// #include <iostream>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <string.h>

#include "Server.hpp"

int main(void)
{
	Server server(8080, 5);

	server.run();

	return 0;
}

// int main(void)
// {
// 	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (serverSocket < 0)
// 	{
// 		std::cerr << "Error: socket creation failed" << std::endl;
// 		return 1;
// 	}

// 	// int opt = 1;

// 	// if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
// 	// {
// 	// 	std::cerr << "Error: setsockopt failed" << std::endl;
// 	// 	return 1;
// 	// }

// 	struct sockaddr_in addr;
// 	socklen_t addrlen = sizeof(addr);

// 	addr.sin_family = AF_INET;
// 	addr.sin_addr.s_addr = INADDR_ANY;
// 	addr.sin_port = htons(8080);

// 	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
// 	{
// 		std::cerr << "Error: bind failed" << std::endl;
// 		return 1;
// 	}

// 	if (listen(serverSocket, 1) < 0)
// 	{
// 		std::cerr << "Error: listen failed" << std::endl;
// 		return 1;
// 	}

// 	while (1)
// 	{
// 		int clientSocket = accept(serverSocket, (struct sockaddr *)&addr, &addrlen);
// 		if (clientSocket < 0)
// 		{
// 			std::cerr << "Error: accept failed" << std::endl;
// 			return 1;
// 		}

// 		char buffer[1024] = {0};
// 		if (recv(clientSocket, buffer, 1024, 0) < 0)
// 		{
// 			std::cerr << "Error: recv failed" << std::endl;
// 			return 1;
// 		}
// 		std::cout << buffer << std::endl;
// 		close(clientSocket);
// 		usleep(1000000);
// 	}

// 	close(serverSocket);

// 	return 0;
// }
