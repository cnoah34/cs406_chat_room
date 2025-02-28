#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "commonFunctions.hpp"

using json = nlohmann::json;


void getMessages(httplib::Response& res, ChatRoomDB& database, const std::string room_id, const std::string start_date, const std::string end_date) {
    if (room_id.empty() || start_date.empty() || end_date.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    const std::string query("SELECT user_id, content, created_at FROM chat.messages "
            "WHERE room_id = ? AND created_at >= minTimeuuid(?) "
            "AND created_at <= maxTimeuuid(?);");

    const json result = database.SelectQuery(query.c_str(), {room_id, start_date, end_date});

    if (result.empty()) {
        res.status = 404;
        res.set_content(R"({"error", "No messages found"})", "application/json");
        return;
    }

    res.status = 200;
    res.set_content(result.dump(), "application/json");
    return;
}

void deleteMessage(httplib::Response& res, ChatRoomDB& database, const std::string room_id, const std::string created_at) {
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
    if (!checkFields(req, res, {"room_id", "user_id", "content"})) {
        return;
    }

    const json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];
    const std::string content = body["content"];

    const std::string query_username("SELECT username FROM chat.users WHERE user_id = ?;");

    const json username_result = database.SelectQuery(query_username.c_str(), {user_id});

    if (!username_result[0].contains("username")) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const std::string username = username_result[0]["username"];

    const std::string verify_room("SELECT COUNT(*) FROM chat.rooms WHERE room_id = ? AND user_ids CONTAINS ?;");
    const json member_result = database.SelectQuery(verify_room.c_str(), {room_id, user_id});

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

    CassUuid room_uuid;
    CassUuid user_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
        cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }
    
    const char* add_message = "INSERT INTO chat.messages (room_id, user_id, username, content, created_at) VALUES (?, ?, ?, ?, now());";
    
    CassStatement* statement = cass_statement_new(add_message, 4);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_uuid(statement, 1, user_uuid);
    cass_statement_bind_string(statement, 2, username.c_str());
    cass_statement_bind_string(statement, 3, content.c_str());

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void defineMessageMethods(httplib::Server& svr, ChatRoomDB& database) {
    svr.Get("/messages", [&database](const httplib::Request& req, httplib::Response res) {
        const std::string room_id = req.get_param_value("room_id");
        const std::string start_date = req.get_param_value("start_date");
        const std::string end_date = req.get_param_value("end_date");
        
        getMessages(res, database, room_id, start_date, end_date);
        setCommonHeaders(res);
    });

    svr.Delete(R"(/messages/:room_id/:created_at)", [&database](const httplib::Request& req, httplib::Response& res) {
        const std::string room_id = req.get_param_value("room_id");
        const std::string created_at = req.get_param_value("created_at");

        deleteMessage(res, database, room_id, created_at);
        setCommonHeaders(res);
    });
    
    svr.Post("/messages", [&database](const httplib::Request& req, httplib::Response& res) {
        createMessage(req, res, database);
        setCommonHeaders(res);
    });

    return;
}

#endif

