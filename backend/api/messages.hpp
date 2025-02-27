#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json get_messages(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string min_date = fields["min_date"];
    const std::string max_date = fields["max_date"];

    const std::string query("SELECT user_id, content, created_at FROM chat.messages "
            "WHERE room_id = " + room_id + " AND created_at >= minTimeuuid('" + min_date + "') "
            "AND created_at <= maxTimeuuid('" + max_date + "');");

    const json result = database.SelectQuery(query.c_str());

    if (result.empty()) {
        return{{"error", "No messages found"}};
    }

    return result;
}

json delete_message(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string created_at = fields["created_at"];

    const std::string query("DELETE FROM chat.messages WHERE room_id = " + room_id + " AND created_at = " + created_at + ";");

    if(!database.ModifyQuery(query.c_str())) {
        return {{"error", "Could not delete message"}};
    }

    return {};
}

json create_message(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];
    const std::string content = fields["content"];

    const std::string query_username("SELECT username FROM chat.users WHERE user_id = " + user_id + ";");

    const json username_result = database.SelectQuery(query_username.c_str());

    if (!username_result[0].contains("username")) {
        return {{"error", "Could not fetch username"}};
    }

    const std::string username = username_result[0]["username"];

    const std::string verify_room("SELECT COUNT(*) FROM chat.rooms WHERE room_id = " + room_id + " AND user_ids CONTAINS " + user_id + ";");

    const json member_result = database.SelectQuery(verify_room.c_str());

    if (!member_result[0].contains("count") || member_result[0]["count"] != 1) {
        return {{"error", "User is not a member of the room"}};
    }
    
    const std::string add_message("INSERT INTO chat.messages (room_id, user_id, username, content, created_at) "
            "VALUES (" + room_id + ", " + user_id + ", '" + username + "', '" + content + "', now());");
    
    if (!database.ModifyQuery(add_message.c_str())) {
        return {{"error", "Could not create message"}};
    }

    return {};
}


#endif

