#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

#include <commonFunctions.hpp>
#include <webSocketManager.hpp>


json getMessages(ChatRoomDB& database, const CassUuid& user_uuid, const CassUuid& room_uuid, const std::string before, const std::string after, const int limit) {
    if (!userInRoom(database, user_uuid, room_uuid)) {
        return { 
            { "error", "Not authorized" }, 
            { "code", 401 } 
        };
    }

    // Most recent messages by default, if user provided then convert to cassandra timestamp
    cass_int64_t before_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) * 1000;
    cass_int64_t after_timestamp = 0;

    if ((!before.empty() && cassTimestampFromString(before.c_str(), &before_timestamp) != CASS_OK) ||
        (!after.empty() && cassTimestampFromString(after.c_str(), &after_timestamp) != CASS_OK)) {
        return { 
            { "error", "Invalid timestamp in parameters" }, 
            { "code", 400 } 
        };
    }

    after_timestamp += 1000; // Add a second to avoid returning same message

    const char* query = "SELECT user_id, username, content, toTimestamp(created_at) AS created_at FROM chat.messages "
        "WHERE room_id = ? AND created_at < minTimeuuid(?) AND created_at > maxTimeuuid(?)"
        "ORDER BY created_at DESC LIMIT ?;";

    CassStatement* statement = cass_statement_new(query, 4);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_int64(statement, 1, before_timestamp);
    cass_statement_bind_int64(statement, 2, after_timestamp);
    cass_statement_bind_int32(statement, 3, limit);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        return { 
            { "error", "No messages found" }, 
            { "code", 404 } 
        };
    }

    return result;
}

/*
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
*/

json createMessage(ChatRoomDB& database, json& message, const CassUuid& user_uuid, const CassUuid& room_uuid, const std::string& content) {
    if (!userInRoom(database, user_uuid, room_uuid)) {
        return { 
            { "error", "Not authorized" }, 
            { "code", 401 } 
        };
    }

    const char* select_username = "SELECT username FROM chat.users WHERE user_id = ?;";
    CassStatement* username_statement = cass_statement_new(select_username, 1);
    cass_statement_bind_uuid(username_statement, 0, user_uuid);

    const json username_result = database.SelectQuery(username_statement);

    if (username_result.empty() || !username_result[0].contains("username")) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    const std::string username = username_result[0]["username"];

    const char* add_message = "INSERT INTO chat.messages (room_id, user_id, username, content, created_at) VALUES (?, ?, ?, ?, now());";
    
    CassStatement* insert_statement = cass_statement_new(add_message, 4);
    cass_statement_bind_uuid(insert_statement, 0, room_uuid);
    cass_statement_bind_uuid(insert_statement, 1, user_uuid);
    cass_statement_bind_string(insert_statement, 2, username.c_str());
    cass_statement_bind_string(insert_statement, 3, content.c_str());

    if (!database.ModifyQuery(insert_statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    const char* time_query = "SELECT dateof(now()) AS time FROM system.local;";
    CassStatement* time_statement = cass_statement_new(time_query, 0);
    json result = database.SelectQuery(time_statement);

    if (result[0].contains("time")) {
        message["created_at"] = result[0]["time"];
        message["username"] = username;
    }

    return {};
}

void defineMessageMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Get("/messages/:room_id", [&database](const httplib::Request& req, httplib::Response& res) {
        std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

        const std::string room_id = req.path_params.at("room_id");
        CassUuid room_uuid;

        if (room_id.empty() || cass_uuid_from_string(room_id.c_str(), &room_uuid ) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        // Get optional query parameters
        std::string before = req.has_param("before") && req.get_param_value("before") != ""
            ? req.get_param_value("before")
            : "";

        std::string after = req.has_param("after") && req.get_param_value("after") != ""
            ? req.get_param_value("after")
            : "";

        int limit = req.has_param("limit") && req.get_param_value("limit") != "" 
            ? std::stoi(req.get_param_value("limit")) 
            : 20;

        json result = getMessages(database, user_uuid, room_uuid, before, after, limit);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 200;
            res.set_content(result.dump(), "application/json");
        }

        setCommonHeaders(res);
    });

    /*
    svr.Delete("/messages/:room_id/:created_at", [&database](const httplib::Request& req, httplib::Response& res) {
        deleteMessage(req, res, database);
        setCommonHeaders(res);
    });
    */
    
    return;
}

void definePostMessage(httplib::Server& svr, ChatRoomDB& database, WebSocketManager& ws_manager) {
    svr.Post("/messages", [&database, &ws_manager](const httplib::Request& req, httplib::Response& res) {
        const std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

        const json body = json::parse(req.body);

        if (!hasFields(body, { "room_id", "content" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];
        const std::string content = body["content"];

        CassUuid room_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
            return;
        }

        json message;

        json result = createMessage(database, message, user_uuid, room_uuid, content);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            // Broadcast message via websocket to connected users
            if (message.contains("created_at") && message.contains("username")) {
                char user_id_str[CASS_UUID_STRING_LENGTH];
                cass_uuid_string(user_uuid, user_id_str);
                message["user_id"] = user_id_str;
                message["content"] = content;

                ws_manager.broadcastToRoom(room_id, message.dump());
            }

            res.status = 204;
        }

        setCommonHeaders(res);
    });

    return;
}

#endif

