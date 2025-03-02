#ifndef CHATUSER_HPP_INCLUDED
#define CHATUSER_HPP_INCLUDED

#include <jwt-cpp/jwt.h>
#include <libbcrypt/include/bcrypt/BCrypt.hpp>

// Custom
#include <chatDB.hpp>
#include <commonFunctions.hpp>

using json = nlohmann::json;


std::string createJwtToken(std::string user_id) {
    const std::string secret = std::getenv("JWT_SECRET");

    if (secret.empty()) {
        std::cerr << "JWT secret not found in environment variables" << std::endl;
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

std::string getUserIdFromUsername(ChatRoomDB& database, std::string username) {
    const char* query = "SELECT user_id FROM chat.users WHERE username = ?";
    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_string(statement, 0, username.c_str());

    const json result = database.SelectQuery(statement);

    if (result.empty() || !result[0].contains("user_id")) {
        return "";
    }

    return result[0]["user_id"];
}

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

int userExists(httplib::Response& res, ChatRoomDB& database, const std::string username) {
    const char* user_exists_query = "SELECT COUNT(*) FROM chat.users WHERE username = ?;"; 
    CassStatement* user_exists_statement = cass_statement_new(user_exists_query, 1);
    cass_statement_bind_string(user_exists_statement, 0, username.c_str());

    const json user_exists_result = database.SelectQuery(user_exists_statement);

    if (user_exists_result.empty() || !user_exists_result[0].contains("count")) {
        // Failed to determine if user exists
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return -1;  // -1 because we don't know
    }
    else if (user_exists_result[0]["count"] == 0) {
        // User does not exist
        res.status = 401;
        res.set_content(R"({"error": "Incorrect username or password"})", "application/json");
        return 0;
    }

    return 1;
}

void verifyUser(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"username", "password"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string username = body["username"];
    const std::string provided_password = body["password"];

    // No user with username
    if (userExists(res, database, username) != 1) {
        return;
    }
    
    const char* credentials_query = "SELECT password, salt FROM chat.users WHERE username = ?;";
    CassStatement* credentials_statement = cass_statement_new(credentials_query, 1);
    cass_statement_bind_string(credentials_statement, 0, username.c_str());

    const json credentials_result = database.SelectQuery(credentials_statement);

    if (credentials_result.empty() || !credentials_result[0].contains("password") || !credentials_result[0].contains("salt")) {
        // Failed to get user credentials
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    char hash[BCRYPT_HASHSIZE];
    char salt[BCRYPT_HASHSIZE]; 

    // Copy the salt from database
    const std::string salt_string = credentials_result[0]["salt"];
    std::strcpy(salt, salt_string.c_str());

    int ret = bcrypt_hashpw(provided_password.c_str(), salt, hash);
    if (ret != 0) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const std::string hashed_provided_pw(hash); 

    // Compare hash created with provided password to database hash
    if (hashed_provided_pw != credentials_result[0]["password"]) {
        // Password does not match
        res.status = 401;
        res.set_content(R"({"error": "Incorrect username or password"})", "application/json");
        return;
    }

    // Create a JWT
    std::string token = createJwtToken(getUserIdFromUsername(database, username));

    if (token.empty()) {
        res.status = 500;
        res.set_content(R"({"error": "Failed to create JWT token"})", "application/json");
        return;
    }
    
    res.status = 200;
    res.set_header("Content-Type", "application/json");
    res.body = "{\"token\":\"" + token + "\"}";
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

    // User with username already exists
    if (userExists(res, database, username) != 0) {
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

