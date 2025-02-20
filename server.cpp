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

// Custom
#include "chatDB.hpp"

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

bool verify_user(ChatRoomDB &database, json fields) {
    std::string username = fields["username"];
    std::string provided_password = fields["password"];

    std::string query = "SELECT password, salt FROM chat.users WHERE username = '" + username + "';";
    json login = database.SelectQuery(query.c_str());

    if (login.empty() || !login[0].contains("password") || !login[0].contains("salt")) {
        std::cout << "Error: could not fetch password/salt for username: " + username << std::endl;
        return false;
    }
    else {
        char hash[BCRYPT_HASHSIZE];
        char salt[BCRYPT_HASHSIZE]; 

        std::string salt_string = login[0]["salt"];

        std::strcpy(salt, salt_string.c_str());

        int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
        if(ret != 0)throw std::runtime_error{"bcrypt: can not generate hash"};
        
        std::string hashed_provided_pw(hash); 

        if (hashed_provided_pw == login[0]["password"]) {
            return true;
        }
    }

    return false;
}

void create_user(ChatRoomDB &database, json fields) {
    std::string username = fields["username"];
    std::string password = fields["password"];

    std::string query = "SELECT COUNT(*) FROM chat.users WHERE username = '" + username + "';"; 
    json check_username = database.SelectQuery(query.c_str());

    if (check_username.empty() || !check_username[0].contains("count") || check_username[0]["count"] != 0) {
        std::cout << "Error: user with username '" + username + "' already exists" << std::endl;
        return;
    }
    else {
        char salt[BCRYPT_HASHSIZE];
        char hash[BCRYPT_HASHSIZE];
        int ret;
        ret = bcrypt_gensalt(12, salt);
        if(ret != 0)throw std::runtime_error{"bcrypt: can not generate salt"};
        ret = bcrypt_hashpw(password.c_str(), salt, hash);
        if(ret != 0)throw std::runtime_error{"bcrypt: can not generate hash"};

        std::string hashed_pw(hash);

        std::string insert_query = 
            "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
            "VALUES (uuid(), toTimestamp(now()), '" + username + "', '" + hashed_pw + "', '" + salt + "');";

        database.SelectQuery(insert_query.c_str());
    }

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
    //database.SelectQuery("SELECT * FROM chat.messages WHERE room_id=0;");
    //database.SelectQuery("SELECT * FROM chat.messages;");
    //json result = database.SelectQuery("SELECT * FROM chat.users;");
    //std::cout << result.dump(3) << std::endl;

    json fields;
    fields["username"] = "newUser";
    fields["password"] = "badPword42";
    //create_user(database, fields);

    std::cout << verify_user(database, fields) << std::endl;

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
