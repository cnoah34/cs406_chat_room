#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


bool add_user_to_room(ChatRoomDB &database, json fields) {
    if (!fields.contains("room_id") || !fields.contains("user_id")) {
        return false;
    }

    std::string room_id = fields["room_id"];
    std::string user_id = fields["user_id"];

    std::string add_user("UPDATE chat.rooms SET user_ids = user_ids + {" + user_id + "} WHERE room_id = " + room_id + ";");

    return database.ModifyQuery(add_user.c_str());
}

bool create_room(ChatRoomDB &database, json fields) {
    if(!fields.contains("name") || !fields.contains("user_id")) {
        return false;
    }

    std::string name = fields["name"];
    std::string user_id = fields["user_id"];

    std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '" + name + "', " + user_id + ", {" + user_id + "}, {" + user_id + "}, toTimestamp(now()));");

    return database.ModifyQuery(add_room.c_str());
}


#endif
