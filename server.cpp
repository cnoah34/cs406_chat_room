#include <iostream>
#include <string> 
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <sys/epoll.h>
#include <sstream>

#include <netinet/in.h>
#include <algorithm>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    listen(serverSocket, 5);

    std::cout << "Server is listening on port " << PORT << ". Press CTRL+C to stop." << std::endl;

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        std::cerr << "Failure to create epoll" << std::endl;
        close(serverSocket);
        return -1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serverSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "Failed to add serverSocket to epoll" << std::endl;
        close(serverSocket);
        close(epoll_fd);
        return -1;
    }

    std::vector<epoll_event> events(MAX_EVENTS);
    std::vector<int> clients;

    while (1) {
        int numEvents = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        
        if (numEvents == -1) {
            std::cerr << "epoll_wait error" << std::endl;
            break;
        }

        for (int i = 0; i < numEvents; i++) {
            int currentSocket = events[i].data.fd;

            // Initial connection
            if (currentSocket == serverSocket) {
                int clientSocket = accept(serverSocket, nullptr, nullptr);

                if (clientSocket == -1) {
                    std::cerr << "Failed to accept client connection" << std::endl;
                    continue;
                }
                
                std::cout << "New client connected: " << clientSocket << std::endl;

                epoll_event clientEvent{};
                clientEvent.events = EPOLLIN;
                clientEvent.data.fd = clientSocket;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &clientEvent) == -1) {
                    std::cerr << "Failed to add client socket to epoll" << std::endl;
                    close(clientSocket);
                }

                clients.push_back(clientSocket);
                std::string welcomeMsg = "Welcome to the server!\n";
                send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);
            }
            else {
                // Message from client
                char buffer[BUFFER_SIZE] = {0};

                int bytesReceived = recv(currentSocket, buffer, sizeof(buffer) - 1, 0);
        
                if (bytesReceived <= 0) {
                    // Client disconnected, close socket and remove from set
                    std::cout << "Client disconnected: " << currentSocket << std::endl;

                    clients.erase(
                        std::remove(clients.begin(), clients.end(), currentSocket),
                        clients.end()
                    );

                    close(currentSocket);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, currentSocket, nullptr);
                }
                else {
                    std::cout << "Received from client " << currentSocket << ": " << buffer << std::endl;

                    std::string message = "Client " + std::to_string(currentSocket) + ": " + buffer;

                    for (int otherClientSocket : clients) {
                        // Send to connected clients that are not the server and message sender
                        if (otherClientSocket != currentSocket) {
                            std::cout << "Sending message to client " << otherClientSocket << std::endl;
                            send(otherClientSocket, message.c_str(), sizeof(message) + 1, 0);
                        }
                    }
                }
            }
        }


        //close(clientSocket);
    }

    // Interrupt handling for later
    /*
    close(serverSocket);

    std::cout << "Server shutdown with no issues." << std::endl;
    */

    return 0;
}
