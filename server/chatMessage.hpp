#ifndef CHATMESSAGE_HPP_INCLUDED
#define CHATMESSAGE_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


bool create_message(ChatRoomDB &database, json fields) {
    if (!fields.contains("room_id") || !fields.contains("user_id") || !fields.contains("content")) {
        return false;
    }

    std::string room_id = fields["room_id"];
    std::string user_id = fields["user_id"];
    std::string content = fields["content"];

    std::string verify_room("SELECT COUNT(*) FROM chat.rooms WHERE room_id = " + room_id + " AND user_ids CONTAINS " + user_id + ";");

    json result = database.SelectQuery(verify_room.c_str());

    if (!result[0].contains("count") || result[0]["count"] != 1) {
        std::cout << "User is not a member of the room" << std::endl;
        return false;
    }
    
    std::string add_message("INSERT INTO chat.messages (room_id, user_id, content, created_at) "
            "VALUES (" + room_id + ", " + user_id + ", '" + content + "', now());");
    
    return database.ModifyQuery(add_message.c_str());
}


#endif

