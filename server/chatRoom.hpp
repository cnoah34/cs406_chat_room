#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json remove_admin_from_user(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string remove_admin("UPDATE chat.rooms SET admin_ids = admin_ids - {" + user_id + "} WHERE room_id = " + room_id + ";");

    if (database.ModifyQuery(remove_admin.c_str())) {
        response["success"] = true;
    }

    return response;
}

json make_user_admin(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];
    
    const std::string make_admin("UPDATE chat.rooms SET admin_ids = admin_ids + {" + user_id + "} WHERE room_id = " + room_id + ";");

    if (database.ModifyQuery(make_admin.c_str())) {
        response["success"] = true;
    }

    return response;
}

json remove_user_from_room(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;
    
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids - {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.rooms SET admin_ids = admin_ids - {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.users SET room_ids = room_ids - {" + room_id + "} WHERE user_id = " + user_id + "; "
            "APPLY BATCH;";

    if(database.ModifyQuery(batch_update.c_str())) {
        response["success"] = true;
    }

    return response;
}

json add_user_to_room(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id", "user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids + {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.users SET room_ids = room_ids + {" + room_id + "} WHERE user_id = " + user_id + "; "
            "APPLY BATCH;";

    if (database.ModifyQuery(batch_update.c_str())) {
        response["success"] = true;
    }


    return response;
}

json delete_room(ChatRoomDB& database, json& fields) {
    std::vector<std::string> required_fields = {"room_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string name = fields["name"];
    const std::string user_id = fields["user_id"];

    const std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '" + name + "', " + user_id + ", {" + user_id + "}, {" + user_id + "}, toTimestamp(now()));");

    if(database.ModifyQuery(add_room.c_str())) {
        response["success"] = true;
    }

    return response;
}

json create_room(ChatRoomDB &database, json& fields) {
    std::vector<std::string> required_fields = {"name", "user_id"};

    json response = check_fields(fields, required_fields);

    if (!response.is_null()) {
        return response;
    }

    response["success"] = false;

    const std::string name = fields["name"];
    const std::string user_id = fields["user_id"];

    const std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '" + name + "', " + user_id + ", {" + user_id + "}, {" + user_id + "}, toTimestamp(now()));");

    if(database.ModifyQuery(add_room.c_str())) {
        response["success"] = true;
    }

    return response;
}


#endif
