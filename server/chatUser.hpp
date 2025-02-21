#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


json verify_user(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if (!fields.contains("username") || !fields.contains("password")) {
        json missing_fields = json::array();
        if (!fields.contains("username")) missing_fields.push_back("username");
        if (!fields.contains("password")) missing_fields.push_back("password");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string username = fields["username"];
    std::string provided_password = fields["password"];

    std::string query = "SELECT password, salt FROM chat.users WHERE username = '" + username + "';";
    json login = database.SelectQuery(query.c_str());

    if (login.empty() || !login[0].contains("password") || !login[0].contains("salt")) {
        response["error"] = "Could not fetch credentials for user"; 
        return response;
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    std::string salt_string = login[0]["salt"];

    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        throw std::runtime_error{"bcrypt: can not generate hash"};
        response["error"] = "Could not generate hash";
        return response;
    }

    std::string hashed_provided_pw(hash); 

    if (hashed_provided_pw == login[0]["password"]) {
        response["success"] = true;
    }

    return response;
}

json create_user(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if (!fields.contains("username") || !fields.contains("password")) {
        json missing_fields = json::array();
        if (!fields.contains("username")) missing_fields.push_back("username");
        if (!fields.contains("password")) missing_fields.push_back("password");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string username = fields["username"];
    std::string password = fields["password"];

    std::string query = "SELECT COUNT(*) FROM chat.users WHERE username = '" + username + "';"; 
    json check_username = database.SelectQuery(query.c_str());

    if (check_username.empty() || !check_username[0].contains("count") || check_username[0]["count"] != 0) {
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

    std::string hashed_pw(hash);

    std::string insert_query = 
        "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
        "VALUES (uuid(), toTimestamp(now()), '" + username + "', '" + hashed_pw + "', '" + salt + "');";

    if(database.ModifyQuery(insert_query.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif

