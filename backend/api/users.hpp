#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

#include <libbcrypt/include/bcrypt/BCrypt.hpp>

#include <commonFunctions.hpp>


std::string createJwtToken(std::string user_id) {
    const char* secret_cstr = std::getenv("JWT_SECRET");
    if (!secret_cstr) {
        std::cerr << "JWT secret not found in environment variables" << std::endl;
        return "";
    }

    std::string secret(secret_cstr);
    if (secret.empty()) {
        return "";
    }

    const auto time = jwt::date::clock::now();

    auto token = jwt::create()
        .set_issuer("chat_rooms")
        .set_subject(user_id)
        .set_payload_claim("user_id", jwt::claim(user_id))
        .set_expires_at(time + std::chrono::hours(24))
        .sign(jwt::algorithm::hs256{secret});

    return token; 
}

std::string getUserIdFromUsername(ChatRoomDB& database, const std::string username) {
    const char* query = "SELECT user_id FROM chat.users WHERE username = ?";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_string(statement, 0, username.c_str());

    const json result = database.SelectQuery(statement);

    if (result.empty() || !result[0].contains("user_id")) {
        return "";
    }

    return result[0]["user_id"];
}

int userExists(ChatRoomDB& database, const std::string& username) {
    const char* user_exists_query = "SELECT COUNT(*) FROM chat.users WHERE username = ?;"; 
    CassStatement* user_exists_statement = cass_statement_new(user_exists_query, 1);
    cass_statement_bind_string(user_exists_statement, 0, username.c_str());

    const json user_exists_result = database.SelectQuery(user_exists_statement);

    if (user_exists_result.empty() || !user_exists_result[0].contains("count")) {
        // Failed to determine if user exists
        return -1;
    }
    else if (user_exists_result[0]["count"] == 0) {
        // User does not exist
        return 0;
    }

    // User exists
    return 1;
}

json getUserDetails(ChatRoomDB& database, const CassUuid& user_uuid) {
    const char* query = "SELECT username, room_ids, created_at FROM chat.users WHERE user_id = ?;"; 

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, user_uuid);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        return { 
            { "error", "No user found" }, 
            { "code", 404 } 
        };
    }

    return result;
}

json verifyUser(ChatRoomDB& database, const std::string& username, const std::string& provided_password) {
    if (userExists(database, username) != 1) {
        return { 
            { "error", "Incorrect username or password" }, 
            { "code", 401 } 
        };
    }
    
    const char* credentials_query = "SELECT password, salt FROM chat.users WHERE username = ?;";
    CassStatement* credentials_statement = cass_statement_new(credentials_query, 1);
    cass_statement_bind_string(credentials_statement, 0, username.c_str());

    const json credentials_result = database.SelectQuery(credentials_statement);

    // Failed to get user credentials
    if (credentials_result.empty() || !credentials_result[0].contains("password") || !credentials_result[0].contains("salt")) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    // Copy the salt from database
    const std::string salt_string = credentials_result[0]["salt"];
    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    const std::string hashed_provided_pw(hash); 

    // Compare hash created with provided password to database hash
    if (hashed_provided_pw != credentials_result[0]["password"]) {
        return { 
            { "error", "Incorrect username or password" }, 
            { "code", 401 } 
        };
    }

    // Create a JWT
    const std::string token = createJwtToken(getUserIdFromUsername(database, username));

    if (token.empty()) {
        return { 
            { "error", "Failed to create token" }, 
            { "code", 500 } 
        };
    }
    
    return {{ "token", token }};
}

json deleteUser(ChatRoomDB& database, const CassUuid user_uuid) { 
    const char* query = "DELETE FROM chat.users WHERE user_id = ?;";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, user_uuid);

    if (!database.ModifyQuery(statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

json createUser(ChatRoomDB& database, const std::string& username, const std::string& password) {
    if (userExists(database, username) != 0) {
        return { 
            { "error", "User with same username exists" }, 
            { "code", 400 } 
        };
    }

    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    int ret;

    ret = bcrypt_gensalt(12, salt);
    if (ret != 0) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    ret = bcrypt_hashpw(password.c_str(), salt, hash);
    if (ret != 0) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    const char* insert_query = 
        "INSERT INTO chat.users (user_id, created_at, username, password, salt) "
        "VALUES (uuid(), toTimestamp(now()), ?, ?, ?);";

    CassStatement* insert_statement = cass_statement_new(insert_query, 3);
    cass_statement_bind_string(insert_statement, 0, username.c_str());
    cass_statement_bind_string(insert_statement, 1, hash);
    cass_statement_bind_string(insert_statement, 2, salt);

    if (!database.ModifyQuery(insert_statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

void defineUserMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Get("/users/metadata", [&database](const httplib::Request& req, httplib::Response& res) {
        std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

        json result = getUserDetails(database, user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 200;
            res.set_content(result[0].dump(), "application/json");
        }

        setCommonHeaders(res);
    });

    svr.Post("/users", [&database](const httplib::Request& req, httplib::Response& res) {
        const json body = json::parse(req.body);

        if (!hasFields(body, { "username", "password" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string username = body["username"];
        const std::string password = body["password"];

        json result = createUser(database, username, password);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }

        setCommonHeaders(res);
    });

    svr.Post("/login", [&database](const httplib::Request& req, httplib::Response& res) {
        const json body = json::parse(req.body);

        if (!hasFields(body, { "username", "password" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string username = body["username"];
        const std::string provided_password = body["password"];

        json result = verifyUser(database, username, provided_password);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            const std::string token = result["token"];

            res.status = 200;
            res.set_header("Content-Type", "application/json");
            res.body = "{\"token\":\"" + token + "\"}";
        }

        setCommonHeaders(res);
    });

    svr.Delete("/users", [&database](const httplib::Request& req, httplib::Response& res) {
        std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

        json result = deleteUser(database, user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }

        setCommonHeaders(res);
    });

    return;
}


#endif

