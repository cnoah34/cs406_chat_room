#include <chatDB.hpp>
#include <json.hpp>

#include <users.hpp>

#define DATABASE_IP "172.19.0.2"

using json = nlohmann::json;

void setCommonHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");

    return;
}

void defineMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        (void)req;  // Avoids unused variable error
        setCommonHeaders(res);
        res.status = 204;
    });

    svr.Post("/users", [&database](const httplib::Request& req, httplib::Response& res) {
        createUser(req, res, database);
        setCommonHeaders(res);
    });

    return;
}
