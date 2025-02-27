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
#include <chatAPI.hpp>

#define PORT 8080
#define DATABASE_IP "172.19.0.2"

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    ChatRoomDB database(DATABASE_IP);

    defineMethods(svr, database);

    svr.listen("0.0.0.0", PORT);

    return 0;
}
