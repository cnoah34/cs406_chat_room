#include <iostream>
#include <string> 
#include <sys/socket.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>
#include <sstream>
#include <algorithm>

// Custom
#include "chatDB.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10
#define DATABASE_IP "172.19.0.2"


void sendToClients(std::vector<int>& clients, std::string message) {
    // Distribute the message to clients in the list
    for (int clientSocket : clients) {
        // Send to connected clients that are not the server and message sender
        std::cout << "Sending message to client " << clientSocket << std::endl;
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    return;
}


void addClient(int serverSocket, std::vector<int>& clients, int epoll_fd) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == -1) {
        std::cerr << "Failed to accept client connection" << std::endl;
        return;
    }

    std::cout << "New client connected: " << clientSocket << std::endl;

    epoll_event clientEvent{};
    clientEvent.events = EPOLLIN;
    clientEvent.data.fd = clientSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &clientEvent) == -1) {
        std::cerr << "Failed to add client socket to epoll" << std::endl;
        close(clientSocket);
    }

    // Create a list of recipients before adding the new client
    std::vector<int> recipients = clients;

    clients.push_back(clientSocket);

    std::string welcomeMessage = "Welcome to the server!\n";
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    // Notify other clients that the new client has joined
    std::string message = "Client " + std::to_string(clientSocket) + " connected";

    sendToClients(recipients, message);

    return;
}


// Message from client
void handleMessage(int senderSocket, std::vector<int>& clients, int epoll_fd) {
    std::string message = "Client " + std::to_string(senderSocket);

    // Create a list of recipients for the message
    std::vector<int> recipients = clients;
    recipients.erase(
            std::remove(recipients.begin(), recipients.end(), senderSocket),
            recipients.end()
            );

    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived = recv(senderSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        // Client disconnected, close socket and remove from set
        std::cout << "Client disconnected: " << senderSocket << std::endl;

        // Remove the disconnected client from the clients list
        clients = recipients;

        close(senderSocket);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, senderSocket, nullptr);

        message += " disconnected";
    }
    else {
        std::cout << "Received from client " << senderSocket << ": " << buffer << std::endl;
        message += ": ";
        message += buffer;
    }

    sendToClients(recipients, message);

    return;
}


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

    // Connect to the ScyllaDB database
    ChatRoomDB database(DATABASE_IP);
    //database.Query("SELECT * FROM chat.messages WHERE room_id=0;");
    //database.Query("SELECT * FROM chat.messages;");
    database.Query("SELECT * FROM chat.users;");

    while (1) {
        int numEvents = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        
        if (numEvents == -1) {
            std::cerr << "epoll_wait error" << std::endl;
            break;
        }

        for (int i = 0; i < numEvents; i++) {
            // Select the socket from which an event was received
            int currentSocket = events[i].data.fd;

            // Initial connection (event is coming from the server socket)
            if (currentSocket == serverSocket) {
                addClient(currentSocket, clients, epoll_fd);
            }
            else {
                // Communication received from a client socket
                handleMessage(currentSocket, clients, epoll_fd);
            }
        }
    }

    // Interrupt handling for later
    /*
    close(serverSocket);

    std::cout << "Server shutdown with no issues." << std::endl;
    */

    return 0;
}
