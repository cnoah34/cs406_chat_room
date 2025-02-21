#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


json get_messages(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if (!fields.contains("room_id") || !fields.contains("created_at")) {
        json missing_fields = json::array();
        if (!fields.contains("room_id")) missing_fields.push_back("room_id");
        if (!fields.contains("created_at")) missing_fields.push_back("created_at");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string room_id = fields["room_id"];
    std::string created_at = fields["created_at"];

    std::string query("SELECT user_id, content, toTimestamp(created_at) AS created_at FROM chat.messages "
            "WHERE room_id = " + room_id + " AND created_at >= minTimeuuid('" + created_at + "');");

    json messages = database.SelectQuery(query.c_str());

    if (messages.empty()) {
        response["error"] = "No messages found";
        return response;
    }

    response["success"] = true;
    response["messages"] = messages;

    return response;
}

json create_message(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if (!fields.contains("room_id") || !fields.contains("user_id") || !fields.contains("content")) {
        json missing_fields = json::array();
        if (!fields.contains("room_id")) missing_fields.push_back("room_id");
        if (!fields.contains("user_id")) missing_fields.push_back("user_id");
        if (!fields.contains("content")) missing_fields.push_back("content");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string room_id = fields["room_id"];
    std::string user_id = fields["user_id"];
    std::string content = fields["content"];

    std::string verify_room("SELECT COUNT(*) FROM chat.rooms WHERE room_id = " + room_id + " AND user_ids CONTAINS " + user_id + ";");

    json result = database.SelectQuery(verify_room.c_str());

    if (!result[0].contains("count") || result[0]["count"] != 1) {
        response["error"] = "User is not a member of the room";
        return response;
    }
    
    std::string add_message("INSERT INTO chat.messages (room_id, user_id, content, created_at) "
            "VALUES (" + room_id + ", " + user_id + ", '" + content + "', now());");
    
    if (database.ModifyQuery(add_message.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif

