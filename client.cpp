#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <string.h>
#define PORT 8080

int main(void)
{
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;
	std::string message = "Hello from client";
	char buffer[1024] = {0};
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "Error: socket creation failed" << std::endl;
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "10.12.3.3", &serv_addr.sin_addr) <= 0)
	{
		std::cerr << "Error: invalid address" << std::endl;
		return -1;
	}

	status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (status < 0)
	{
		std::cerr << "Error: connection failed" << std::endl;
		return -1;
	}

	int recv_size;

	while (1) {
		// Send a message to the server
		std::cout << "Client: ";
		std::cin.getline(buffer, (1024 - 1));
		send(client_fd, buffer, strlen(buffer), 0);

		// Receive a reply from the server
		recv_size = recv(client_fd, buffer, (1024 - 1), 0);
		if (recv_size <= 0) {
			puts("Server disconnected");
			break;
		}
		buffer[recv_size] = '\0';
		std::cout << "Server: " << buffer << std::endl;
	}

	// if (send(client_fd, message.c_str(), message.size(), 0) < 0)
	// {
	// 	std::cerr << "Error: send failed" << std::endl;
	// 	return -1;
	// }
	// std::cout << "Hello message sent" << std::endl;
	// if (recv(client_fd, buffer, 1024, 0) < 0)
	// {
	// 	std::cerr << "Error: recv failed" << std::endl;
	// 	return -1;
	// }
	// std::cout << buffer << std::endl;

	// closing the connected socket
	close(client_fd);
	return 0;
}

// int main() {
// 	int sock;
// 	struct sockaddr_in server;
// 	char message[1000], server_reply[2000];
// 	int recv_size;

// 	// Create socket
// 	sock = socket(AF_INET, SOCK_STREAM, 0);
// 	if (sock == -1) {
// 		printf("Could not create socket");
// 		return 1;
// 	}
// 	puts("Socket created");

// 	server.sin_addr.s_addr = inet_addr("127.0.0.1");
// 	server.sin_family = AF_INET;
// 	server.sin_port = htons(8888);

// 	// Connect to the server
// 	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
// 		perror("Connect failed");
// 		return 1;
// 	}
// 	puts("Connected to server");

// 	// Back and forth communication
// 	while (1) {
// 		// Send a message to the server
// 		printf("Client: ");
// 		fgets(message, sizeof(message), stdin);
// 		send(sock, message, strlen(message), 0);

// 		// Receive a reply from the server
// 		recv_size = recv(sock, server_reply, sizeof(server_reply), 0);
// 		if (recv_size <= 0) {
// 			puts("Server disconnected");
// 			break;
// 		}
// 		server_reply[recv_size] = '\0';
// 		printf("Server: %s\n", server_reply);
// 	}

// 	// Close the socket
// 	close(sock);

// 	return 0;
// }
