#include <iostream>
#include <csignal>
#include <string> 
#include <atomic>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>
#include <sstream>
#include <algorithm>

#include <json.hpp>
#include <BCrypt.hpp>
#include <httplib.h>

// Custom
#include "chatDB.hpp"
#include "chatUser.hpp"
#include "chatMessage.hpp"
#include "chatRoom.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10
#define DATABASE_IP "172.19.0.2"


std::atomic<bool> keep_running(true);

void signal_handler(int signal) {
    if (signal == SIGINT) {
        keep_running = false;
        std::cout << "SIGINT received, exiting program" << std::endl;
    }

    return;
}

void send_to_clients(std::vector<int>& clients, std::string message) {
    // Distribute the message to clients in the list
    for (int clientSocket : clients) {
        // Send to connected clients that are not the server and message sender
        std::cout << "Sending message to client " << clientSocket << std::endl;
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    return;
}

void add_client(int serverSocket, std::vector<int>& clients, int epoll_fd) {
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

    send_to_clients(recipients, message);

    return;
}

// Message from client
void handle_message(int senderSocket, std::vector<int>& clients, int epoll_fd) {
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

    send_to_clients(recipients, message);

    return;
}




int main() {
    std::signal(SIGINT, signal_handler);

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

    /* Testing
    json fields;
    fields["username"] = "newUser";
    fields["password"] = "badPword42";

    std::cout << verify_user(database, fields) << std::endl;
    */
   
    json fields;    
    json j;


    //j = create_room(database, fields);

    //json fields["room_id"] = database.SelectQuery("SELECT room_id FROM chat.rooms WHERE name = 'The Ark';");
    

    fields["name"] = "The Ark";
    fields["user_id"] = "18bb225d-2f55-4df6-b09d-7c4420d527f5";
    fields["room_id"] = "df5f37cb-de3a-4887-8cf3-abf6a882c088";

    //std::cout << remove_admin_from_user(database, fields).dump(3) << std::endl;

    //std::cout << add_user_to_room(database, fields).dump(3) << std::endl;
    //std::cout << remove_user_from_room(database, fields).dump(3) << std::endl;

    fields["content"] = "This is a test message!";
    std::cout << create_message(database, fields).dump(3) << std::endl;

    fields["content"] = "Hello world!";
    std::cout << create_message(database, fields).dump(3) << std::endl;

    fields["min_date"] = "2025-02-15 00:00:00";
    fields["max_date"] = "2026-01-01 00:00:00";

    std::cout << get_messages(database, fields)["messages"].dump(3) << std::endl;
    //fields["created_at"] = message["created_at"];

    //std::cout << delete_message(database, fields).dump(3) << std::endl;

    //std::cout << get_username(database, fields).dump(3) << std::endl;

    while (keep_running) {
        int numEvents = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        
        if (numEvents == -1) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "epoll_wait error" << std::endl;
            break;
        }

        for (int i = 0; i < numEvents; i++) {
            // Select the socket from which an event was received
            int currentSocket = events[i].data.fd;

            // Initial connection (event is coming from the server socket)
            if (currentSocket == serverSocket) {
                add_client(currentSocket, clients, epoll_fd);
            }
            else {
                // Communication received from a client socket
                handle_message(currentSocket, clients, epoll_fd);
            }
        }
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, serverSocket, nullptr);
    close(serverSocket);

    std::cout << "Server shutdown with no issues" << std::endl;

    return 0;
}
