#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


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

#endif

