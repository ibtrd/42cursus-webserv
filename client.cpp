#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define SERVER_IP "10.12.3.3"

int client_fd;

std::string get_my_ip() {
	char buffer[1024];
	std::string ip;
	FILE* fp = popen("hostname -I", "r");
	if (fp == NULL) {
		std::cerr << "Error: failed to get IP address" << std::endl;
		return "";
	}
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		ip += buffer;
	}
	pclose(fp);
	// get the first IP address
	ip = ip.substr(0, ip.find(" "));
	return ip;
}

// Function to handle receiving messages from the server
void* receive_messages(void* arg) {
    char buffer[1024];
    int recv_size;
    while (1) {
        // Receive a message from the server
        recv_size = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (recv_size <= 0) {
            std::cout << "Server disconnected or error receiving" << std::endl;
            break;
        }
        buffer[recv_size] = '\0'; // Null-terminate the received message

		// erase line
		std::cout << "\033[2K\r";

        std::cout << buffer << std::endl;

		// print client
		std::cout << (char *)arg << ": " << std::flush;
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int status;
    struct sockaddr_in serv_addr;
    std::string message;
	std::string my_ip;
	std::string server_ip = SERVER_IP;

	if (argc > 1) {
		server_ip = argv[1];
	}

	my_ip = get_my_ip();
	std::cout << "My IP: " << my_ip << std::endl;

    // Create the socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: socket creation failed" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Error: invalid address" << std::endl;
        return -1;
    }

    // Connect to the server
    status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (status < 0) {
        std::cerr << "Error: connection failed" << std::endl;
        return -1;
    }

    // Create a thread for receiving messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void *)my_ip.c_str()) != 0) {
        std::cerr << "Error: failed to create receive thread" << std::endl;
        return -1;
    }

    // Main thread will handle sending messages
    char buffer[1024];
    while (1) {
        std::cout << my_ip << ": ";
        std::cin.getline(buffer, sizeof(buffer) - 1);

		if (std::cin.eof()) {
			pthread_cancel(recv_thread);
			break;
		}

		// Move cursor up
		// std::cout << "\033[A";
		// Erase the line
		// std::cout << "\033[2K";

        // // Erase the line after input
        std::cout << "\033[A\33[2K\r";
		// std::cout << "\033[5C";

        // Exit the loop if "exit" command is given
        if (strcmp(buffer, "exit") == 0) {
			// stop the receiving thread
			pthread_cancel(recv_thread);
            break;
        }
        
        // Send the message to the server
        if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
            std::cerr << "Error: sending message failed" << std::endl;
			pthread_cancel(recv_thread);
            break;
        }
    }

    // Wait for the receiving thread to finish
    pthread_join(recv_thread, NULL);

    // Closing the connected socket
    close(client_fd);
    return 0;
}












// #include <arpa/inet.h>
// #include <iostream>
// #include <sys/socket.h>
// #include <unistd.h>
// #include <string>
// #include <string.h>

// #define PORT 8080



// int main(void)
// {
// 	int status, valread, client_fd;
// 	struct sockaddr_in serv_addr;
// 	std::string message = "Hello from client";
// 	char buffer[1024] = {0};
// 	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
// 	{
// 		std::cerr << "Error: socket creation failed" << std::endl;
// 		return -1;
// 	}

// 	serv_addr.sin_family = AF_INET;
// 	serv_addr.sin_port = htons(PORT);

// 	// Convert IPv4 and IPv6 addresses from text to binary
// 	// form
// 	if (inet_pton(AF_INET, "10.12.3.3", &serv_addr.sin_addr) <= 0)
// 	{
// 		std::cerr << "Error: invalid address" << std::endl;
// 		return -1;
// 	}

// 	status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
// 	if (status < 0)
// 	{
// 		std::cerr << "Error: connection failed" << std::endl;
// 		return -1;
// 	}

// 	int recv_size;

// 	while (1) {
// 		// Send a message to the server
// 		std::cout << "Client: ";
// 		std::cin.getline(buffer, (1024 - 1));
// 		send(client_fd, buffer, strlen(buffer), 0);

// 		// Receive a reply from the server
// 		recv_size = recv(client_fd, buffer, (1024 - 1), 0);
// 		if (recv_size <= 0) {
// 			puts("Server disconnected");
// 			break;
// 		}
// 		buffer[recv_size] = '\0';
// 		std::cout << buffer << std::endl;
// 	}

// 	// closing the connected socket
// 	close(client_fd);
// 	return 0;
// }













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
