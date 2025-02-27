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

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        (void)req;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    svr.Post("/users", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            if (!body.contains("username") || !body["username"].is_string() ||
                !body.contains("password") || !body["password"].is_string()) {
                res.status = 400;
                res.set_content(R"({"error", "Missing or invalid required fields"})", "application/json");
                return;
            }

            ChatRoomDB database(DATABASE_IP);
            json responseBody = create_user(database, body);


            if (responseBody.contains("error")) {
                if (responseBody["error"].get<std::string>().find("already exists") != std::string::npos) {
                    res.status = 400;   // User with username already exists, bad request
                }
                else {
                    res.status = 500;   // Interal server error
                }

                res.set_content(responseBody.dump(), "application/json");
            }
            else {
                res.status = 204;
            }

            res.set_header("Access-Control-Allow-Origin", "*");
        }
        catch (const json::parse_error& e) {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
        }
    });

    svr.listen("0.0.0.0", PORT);

    return 0;
}
