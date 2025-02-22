#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json get_messages(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "min_date", "max_date"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string room_id = fields["room_id"];
    const std::string min_date = fields["min_date"];
    const std::string max_date = fields["max_date"];

    const std::string query("SELECT user_id, content, created_at FROM chat.messages "
            "WHERE room_id = " + room_id + " AND created_at >= minTimeuuid('" + min_date + "') "
            "AND created_at <= maxTimeuuid('" + max_date + "');");

    const json result = database.SelectQuery(query.c_str());

    if (result.empty()) {
        response["error"] = "No messages found";
        return response;
    }

    response["success"] = true;
    response["messages"] = result;

    return response;
}

json delete_message(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "created_at"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;
    
    const std::string room_id = fields["room_id"];
    const std::string created_at = fields["created_at"];

    const std::string query("DELETE FROM chat.messages WHERE room_id = " + room_id + " AND created_at = " + created_at + ";");

    if(database.ModifyQuery(query.c_str())) {
        response["success"] = true;
    }

    return response;
}

json create_message(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "user_id", "content"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];
    const std::string content = fields["content"];

    const std::string query_username("SELECT username FROM chat.users WHERE user_id = " + user_id + ";");

    const json username_result = database.SelectQuery(query_username.c_str());

    if (!username_result[0].contains("username")) {
        response["error"] = "Could not fetch username";
        return response;
    }

    const std::string username = username_result[0]["username"];

    const std::string verify_room("SELECT COUNT(*) FROM chat.rooms WHERE room_id = " + room_id + " AND user_ids CONTAINS " + user_id + ";");

    const json member_result = database.SelectQuery(verify_room.c_str());

    if (!member_result[0].contains("count") || member_result[0]["count"] != 1) {
        response["error"] = "User is not a member of the room";
        return response;
    }
    
    const std::string add_message("INSERT INTO chat.messages (room_id, user_id, username, content, created_at) "
            "VALUES (" + room_id + ", " + user_id + ", '" + username + "', '" + content + "', now());");
    
    if (database.ModifyQuery(add_message.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif

