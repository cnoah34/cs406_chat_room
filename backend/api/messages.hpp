#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

#include <commonFunctions.hpp>


void getMessages(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    std::string authHeader = req.get_header_value("Authorization");

    std::optional<CassUuid> userUuidOpt = getUserIdFromToken(authHeader);
    if (!userUuidOpt.has_value()) {
        res.status = 401;
        res.set_content(R"({"error": "Not authorized"})", "application/json");
        return;
    }

    CassUuid user_uuid = userUuidOpt.value();

    const std::string room_id = req.path_params.at("room_id");

    if (room_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    CassUuid room_uuid;
    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter 'room_id'"})", "application/json");
        return;
    }

    if (!userInRoom(database, user_uuid, room_uuid)) {
        res.status = 401;
        res.set_content(R"({"error": "Not authorized"})", "application/json");
        return;
    }

    std::string before = req.has_param("before") ? req.get_param_value("before") : "";
    std::string limit_str = req.has_param("limit") ? req.get_param_value("limit") : "50";
    int limit = stoi(limit_str);


    // Most recent messages by default
    cass_int64_t before_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) * 1000;
    if (!before.empty() && cassTimestampFromString(before.c_str(), &before_timestamp) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter 'before'"})", "application/json");
        return;
    }

    const char* query = "SELECT user_id, username, content, created_at FROM chat.messages "
        "WHERE room_id = ? AND created_at < minTimeuuid(?) "
        "ORDER BY created_at DESC LIMIT ?;";

    CassStatement* statement = cass_statement_new(query, 3);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_int64(statement, 1, before_timestamp);
    cass_statement_bind_int32(statement, 2, limit);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        res.status = 404;
        res.set_content(R"({"error": "No messages found"})", "application/json");
        return;
    }

    res.status = 200;
    res.set_content(result.dump(), "application/json");
    return;
}

void deleteMessage(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    const std::string room_id = req.path_params.at("room_id");
    const std::string created_at = req.path_params.at("created_at");

    if (room_id.empty() || created_at.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    CassUuid room_uuid;
    CassUuid created_at_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
            cass_uuid_from_string(created_at.c_str(), &created_at_uuid) != CASS_OK) {

        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = "DELETE FROM chat.messages WHERE room_id = ? AND created_at = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_uuid(statement, 1, created_at_uuid);

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error", "Could not delete message"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void createMessage(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    std::string authHeader = req.get_header_value("Authorization");

    std::optional<CassUuid> userUuidOpt = getUserIdFromToken(authHeader);
    if (!userUuidOpt.has_value()) {
        res.status = 401;
        res.set_content(R"({"error": "Not authorized"})", "application/json");
        return;
    }

    CassUuid user_uuid = userUuidOpt.value();

    if (!checkFields(req, res, {"room_id", "content"})) {
        return;
    }

    const json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string content = body["content"];

    CassUuid room_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* select_username = "SELECT username FROM chat.users WHERE user_id = ?;";

    CassStatement* username_statement = cass_statement_new(select_username, 1);
    cass_statement_bind_uuid(username_statement, 0, user_uuid);

    const json username_result = database.SelectQuery(username_statement);

    if (!username_result[0].contains("username")) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const std::string username = username_result[0]["username"];

    const char* verify_query = "SELECT COUNT(*) FROM chat.rooms WHERE room_id = ? AND user_ids CONTAINS ?;";

    CassStatement* verify_statement = cass_statement_new(verify_query, 2);
    cass_statement_bind_uuid(verify_statement, 0, room_uuid);
    cass_statement_bind_uuid(verify_statement, 1, user_uuid);

    const json member_result = database.SelectQuery(verify_statement);

    if (!member_result[0].contains("count")) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }
    else if (member_result[0]["count"] != 1) {
        res.status = 400;
        res.set_content(R"({"error": "User is not a member of the room"})", "application/json");
        return;

    }

    const char* add_message = "INSERT INTO chat.messages (room_id, user_id, username, content, created_at) VALUES (?, ?, ?, ?, now());";
    
    CassStatement* insert_statement = cass_statement_new(add_message, 4);
    cass_statement_bind_uuid(insert_statement, 0, room_uuid);
    cass_statement_bind_uuid(insert_statement, 1, user_uuid);
    cass_statement_bind_string(insert_statement, 2, username.c_str());
    cass_statement_bind_string(insert_statement, 3, content.c_str());

    if (!database.ModifyQuery(insert_statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void defineMessageMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Get("/messages/:room_id", [&database](const httplib::Request& req, httplib::Response& res) {
        getMessages(req, res, database);
        setCommonHeaders(res);
    });

    svr.Delete("/messages/:room_id/:created_at", [&database](const httplib::Request& req, httplib::Response& res) {
        deleteMessage(req, res, database);
        setCommonHeaders(res);
    });
    
    svr.Post("/messages", [&database](const httplib::Request& req, httplib::Response& res) {
        createMessage(req, res, database);
        setCommonHeaders(res);
    });

    return;
}


#endif

