#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "commonFunctions.hpp"

using json = nlohmann::json;


void getUserDetails(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    // Get and verify params
    const std::string user_id = req.path_params.at("user_id");

    if (user_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    CassUuid user_uuid;

    if (cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    // Query
    const char* query = "SELECT username, room_ids, created_at FROM chat.users WHERE user_id = ?;"; 

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, user_uuid);

    const json result = database.SelectQuery(statement);

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

    const char* query = "SELECT password, salt FROM chat.users WHERE username = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_string(statement, 0, username.c_str());

    const json result = database.SelectQuery(statement);

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

void deleteUser(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) { 
    // Verify params
    const std::string user_id = req.path_params.at("user_id");

    if (user_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    CassUuid user_uuid;

    if (cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    // Query
    const char* query = "DELETE FROM chat.users WHERE user_id = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, user_uuid);

    if (!database.ModifyQuery(statement)) {
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

    const char* select_query = "SELECT COUNT(*) FROM chat.users WHERE username = ?;"; 
    
    CassStatement* select_statement = cass_statement_new(select_query, 1);
    cass_statement_bind_string(select_statement, 0, username.c_str());

    const json result = database.SelectQuery(select_statement);

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

    //const std::string hashed_pw(hash);

    const char* insert_query = 
        "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
        "VALUES (uuid(), toTimestamp(now()), ?, ?, ?);";

    CassStatement* insert_statement = cass_statement_new(insert_query, 3);
    cass_statement_bind_string(insert_statement, 0, username.c_str());
    cass_statement_bind_string(insert_statement, 1, hash);
    cass_statement_bind_string(insert_statement, 2, salt);

    if (!database.ModifyQuery(insert_statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void defineUserMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Get("/users/:user_id", [&database](const httplib::Request& req, httplib::Response& res) {
        getUserDetails(req, res, database);
        setCommonHeaders(res);
    });

    svr.Post("/users", [&database](const httplib::Request& req, httplib::Response& res) {
        createUser(req, res, database);
        setCommonHeaders(res);
    });

    svr.Post("/login", [&database](const httplib::Request& req, httplib::Response& res) {
        verifyUser(req, res, database);
        setCommonHeaders(res);
    });

    svr.Delete("/users/:user_id", [&database](const httplib::Request& req, httplib::Response& res) {
        deleteUser(req, res, database);
        setCommonHeaders(res);
    });

    return;
}


#endif

