#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


void getUserDetails(httplib::Response& res, ChatRoomDB& database, const std::string user_id) {
    if (user_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing user ID"})", "application/json");
        return;
    }

    const std::string query = "SELECT username, room_ids, created_at FROM chat.users WHERE user_id = ?;"; 
    const json result = database.SelectQuery(query.c_str(), {user_id});

    if (result.empty()) {
        res.status = 404;
        res.set_content(R"({"error": "User could not be found"})", "application/json");
        return;
    }
    
    res.status = 200;
    res.set_content(result[0].dump(), "application/json");
    return;
}


void verifyUser(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"username", "password"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string username = body["username"];
    const std::string provided_password = body["password"];

    const std::string query = "SELECT password, salt FROM chat.users WHERE username = ?;";

    const json result = database.SelectQuery(query.c_str(), {username});

    if (result.empty() || !result[0].contains("password") || !result[0].contains("salt")) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    // Copy the salt from database
    const std::string salt_string = result[0]["salt"];
    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const std::string hashed_provided_pw(hash); 

    if (hashed_provided_pw != result[0]["password"]) {
        res.status = 401;
        res.set_content(R"({"error": "Incorrect password"})", "application/json");
        return;
    }

    // Create and return JWT here?
    res.status = 204;
    return;
}

void deleteUser(httplib::Response& res, ChatRoomDB& database, const std::string user_id) { 
    if (user_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing user ID"})", "application/json");
        return;
    }

    const std::string query("DELETE FROM chat.users WHERE user_id = ?;");

    if(!database.ModifyQuery(query.c_str(), {user_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }
    
    res.status = 204;
    return;
}

void createUser(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"username", "password"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string username = body["username"];
    const std::string password = body["password"];

    const std::string query = "SELECT COUNT(*) FROM chat.users WHERE username = ?;"; 
    const json result = database.SelectQuery(query.c_str(), {username});

    if (result.empty() || !result[0].contains("count")) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }
    else if (result[0]["count"] != 0) {
        res.status = 400;
        res.set_content(R"({"error": "User with same username already exists"})", "application/json");
        return;
    }

    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    int ret;

    ret = bcrypt_gensalt(12, salt);
    if (ret != 0) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    ret = bcrypt_hashpw(password.c_str(), salt, hash);
    if (ret != 0) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const std::string hashed_pw(hash);

    const std::string insert_query = 
        "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
        "VALUES (uuid(), toTimestamp(now()), ?, ?, ?);";

    if (!database.ModifyQuery(insert_query.c_str(), {username, hashed_pw, salt})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}


#endif

