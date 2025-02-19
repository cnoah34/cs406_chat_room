#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>

#include <thread>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveMessages(int clientSocket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }

        std::cout << buffer << std::endl;
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (!connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) {
        std::cerr << "Failed to connect to the server at port: " <<  PORT << std::endl;
        return -1;
    }

    std::cout << "Connected to the server. Type messages below and use CTRL+C to quit." << std::endl;
    
    std::thread receiverThread(receiveMessages, clientSocket);

    std::string message;

    while (1) {
        //std::cout << "> ";
        std::getline(std::cin, message);

        if (message.empty()) {
            continue;
        }

        int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);

        if (bytesSent == -1) {
            std::cerr << "Failed to send message." << std::endl;
            break;
        }
    }

    close(clientSocket);

    return 0;
}
