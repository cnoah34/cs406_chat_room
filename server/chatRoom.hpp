#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

#include <string> 

#include <json.hpp>
#include <BCrypt.hpp>

// Custom
#include "chatDB.hpp"


json add_user_to_room(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if (!fields.contains("room_id") || !fields.contains("user_id")) {
        json missing_fields = json::array();
        if (!fields.contains("room_id")) missing_fields.push_back("room_id");
        if (!fields.contains("user_id")) missing_fields.push_back("user_id");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string room_id = fields["room_id"];
    std::string user_id = fields["user_id"];

    std::string add_user("UPDATE chat.rooms SET user_ids = user_ids + {" + user_id + "} WHERE room_id = " + room_id + ";");

    if(database.ModifyQuery(add_user.c_str())) {
        response["success"] = true;
    }

    return response;
}

json create_room(ChatRoomDB &database, json fields) {
    json response;
    response["success"] = false;

    if(!fields.contains("name") || !fields.contains("user_id")) {
        json missing_fields = json::array();
        if (!fields.contains("name")) missing_fields.push_back("name");
        if (!fields.contains("user_id")) missing_fields.push_back("user_id");

        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        return response;
    }

    std::string name = fields["name"];
    std::string user_id = fields["user_id"];

    std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '" + name + "', " + user_id + ", {" + user_id + "}, {" + user_id + "}, toTimestamp(now()));");

    if(database.ModifyQuery(add_room.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif
