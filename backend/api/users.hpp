#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json get_username(ChatRoomDB& database, json& fields) {
    const std::string user_id = fields["user_id"];

    const std::string query = "SELECT username FROM chat.users WHERE user_id = " + user_id + ";";

    const json result = database.SelectQuery(query.c_str());

    if (result.empty()) {
        return {{"error", "User could not be found"}};
    }

    return result[0];
}


json verify_user(ChatRoomDB& database, json& fields) {
    const std::string username = fields["username"];
    const std::string provided_password = fields["password"];

    const std::string query = "SELECT password, salt FROM chat.users WHERE username = '" + username + "';";

    const json result = database.SelectQuery(query.c_str());

    if (result.empty() || !result[0].contains("password") || !result[0].contains("salt")) {
        return {{"error", "Internal server error"}};
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    const std::string salt_string = result[0]["salt"];

    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        return {{"error", "Internal server error"}};
    }

    const std::string hashed_provided_pw(hash); 

    if (hashed_provided_pw != result[0]["password"]) {
        return {{"error", "Incorrect password"}};
    }

    return {};
}

json delete_user(ChatRoomDB& database, json& fields) {
    const std::string user_id = fields["user_id"];

    const std::string query("DELETE FROM chat.users WHERE user_id = " + user_id + ";");

    if(!database.ModifyQuery(query.c_str())) {
        return {{"error", "Could not delete user"}};
    }
    
    return {};
}

void createUser(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"username", "password"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string username = body["username"];
    const std::string password = body["password"];

    const std::string query = "SELECT COUNT(*) FROM chat.users WHERE username = '" + username + "';"; 
    const json result = database.SelectQuery(query.c_str());

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
        "VALUES (uuid(), toTimestamp(now()), '" + username + "', '" + hashed_pw + "', '" + salt + "');";

    if (!database.ModifyQuery(insert_query.c_str())) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}


#endif

