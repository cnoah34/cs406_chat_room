#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


json remove_admin_from_user(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string remove_admin("UPDATE chat.rooms SET admin_ids = admin_ids - {" + user_id + "} WHERE room_id = " + room_id + ";");

    if (!database.ModifyQuery(remove_admin.c_str())) {
        return {{"error", "Could not remove admin status from user"}};
    }

    return {};
}

json make_user_admin(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];
    
    const std::string make_admin("UPDATE chat.rooms SET admin_ids = admin_ids + {" + user_id + "} WHERE room_id = " + room_id + ";");

    if (!database.ModifyQuery(make_admin.c_str())) {
        return {{"error", "Could not make user an admin"}};
    }

    return {};
}

json remove_user_from_room(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids - {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.rooms SET admin_ids = admin_ids - {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.users SET room_ids = room_ids - {" + room_id + "} WHERE user_id = " + user_id + "; "
            "APPLY BATCH;";

    if(!database.ModifyQuery(batch_update.c_str())) {
        return {{"error", "Could not remove user from the room"}};
    }

    return {};
}

json add_user_to_room(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];
    const std::string user_id = fields["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids + {" + user_id + "} WHERE room_id = " + room_id + "; "
            "UPDATE chat.users SET room_ids = room_ids + {" + room_id + "} WHERE user_id = " + user_id + "; "
            "APPLY BATCH;";

    if (!database.ModifyQuery(batch_update.c_str())) {
        return {{"error", "Could not add user to the room"}};
    }


    return {};
}

json delete_room(ChatRoomDB& database, json& fields) {
    const std::string room_id = fields["room_id"];

    // SHOULD PROBABLY VERIFY THAT USER IS OWNER HERE

    const std::string delete_room("DELETE FROM chat.rooms WHERE room_id = " + room_id + ";");

    if (!database.ModifyQuery(delete_room.c_str())) {
        return {{"error", "Could not delete the room"}};
    }

    return {};
}

json create_room(ChatRoomDB &database, json& fields) {
    const std::string name = fields["name"];
    const std::string user_id = fields["user_id"];

    const std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '" + name + "', " + user_id + ", {" + user_id + "}, {" + user_id + "}, toTimestamp(now()));");

    if (!database.ModifyQuery(add_room.c_str())) {
        return {{"error", "Could not create the room"}};
    }

    return {};
}


#endif
