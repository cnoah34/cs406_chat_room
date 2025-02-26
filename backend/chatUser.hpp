#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json get_username(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string user_id = fields["user_id"];

    const std::string query = "SELECT username FROM chat.users WHERE user_id = " + user_id + ";";

    const json result = database.SelectQuery(query.c_str());

    if (result.empty()) {
        response["error"] = "No username found";
        return response;
    }

    response["success"] = true;
    response.update(result[0]);

    return response;
}


json verify_user(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"username", "password"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string username = fields["username"];
    const std::string provided_password = fields["password"];

    const std::string query = "SELECT password, salt FROM chat.users WHERE username = '" + username + "';";

    const json result = database.SelectQuery(query.c_str());

    if (result.empty() || !result[0].contains("password") || !result[0].contains("salt")) {
        response["error"] = "Could not fetch credentials for user"; 
        return response;
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    const std::string salt_string = result[0]["salt"];

    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        throw std::runtime_error{"bcrypt: can not generate hash"};
        response["error"] = "Could not generate hash";
        return response;
    }

    const std::string hashed_provided_pw(hash); 

    if (hashed_provided_pw == result[0]["password"]) {
        response["success"] = true;
    }

    return response;
}

json delete_user(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string user_id = fields["user_id"];

    const std::string query("DELETE FROM chat.users WHERE user_id = " + user_id + ";");

    if(database.ModifyQuery(query.c_str())) {
        response["success"] = true;
    }
    
    return response;
}

json create_user(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"username", "password"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string username = fields["username"];
    const std::string password = fields["password"];

    const std::string query = "SELECT COUNT(*) FROM chat.users WHERE username = '" + username + "';"; 

    const json result = database.SelectQuery(query.c_str());

    if (result.empty() || !result[0].contains("count") || result[0]["count"] != 0) {
        response["error"] = "User with username '" + username + "' already exists";
        return response;
    }

    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    int ret;

    ret = bcrypt_gensalt(12, salt);
    if (ret != 0) {
        throw std::runtime_error{"bcrypt: can not generate salt"};
        response["error"] = "Server runtime error";
        return response;
    }

    ret = bcrypt_hashpw(password.c_str(), salt, hash);
    if (ret != 0) {
        throw std::runtime_error{"bcrypt: can not generate hash"};
        response["error"] = "Server runtime error";
        return response;
    }

    const std::string hashed_pw(hash);

    const std::string insert_query = 
        "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
        "VALUES (uuid(), toTimestamp(now()), '" + username + "', '" + hashed_pw + "', '" + salt + "');";

    if (database.ModifyQuery(insert_query.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif

