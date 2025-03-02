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
#include <chatDB.hpp>
#include <users.hpp>
#include <messages.hpp>
#include <rooms.hpp>


using json = nlohmann::json;

int main() {
    const char* portStr = std::getenv("PORT");
    if (portStr == nullptr) {
        std::cerr << "PORT environment variable not set" << std::endl;
        return 1;
    }

    int port = std::stoi(portStr);

    httplib::Server svr;

    ChatRoomDB database(std::getenv("DATABASE_IP"));

    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        setCommonHeaders(res);
        res.status = 204;
    });

    defineUserMethods(svr, database);
    defineMessageMethods(svr, database);
    defineRoomMethods(svr, database);

    svr.listen("0.0.0.0", port);

    return 0;
}
