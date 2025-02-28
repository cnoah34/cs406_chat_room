#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

// Custom
#include "chatDB.hpp"
#include "checkFields.hpp"


void removeAdmin(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    const std::string remove_admin("UPDATE chat.rooms SET admin_ids = admin_ids - {?} WHERE room_id = ?;");

    if (!database.ModifyQuery(remove_admin.c_str(), {user_id, room_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void makeAdmin(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    const std::string make_admin("UPDATE chat.rooms SET admin_ids = admin_ids + {?} WHERE room_id = ?;");

    if (!database.ModifyQuery(make_admin.c_str(), {user_id, room_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void removeUserFromRoom(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids - {?} WHERE room_id = ?; "
            "UPDATE chat.rooms SET admin_ids = admin_ids - {?} WHERE room_id = ?; "
            "UPDATE chat.users SET room_ids = room_ids - {?} WHERE user_id = ?; "
            "APPLY BATCH;";

    if(!database.ModifyQuery(batch_update.c_str(), {user_id, room_id, user_id, room_id, room_id, user_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void addUserToRoom(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);

    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    const std::string batch_update = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids + {?} WHERE room_id = ?; "
            "UPDATE chat.users SET room_ids = room_ids + {?} WHERE user_id = ?; "
            "APPLY BATCH;";

    if (!database.ModifyQuery(batch_update.c_str(), {user_id, room_id, room_id, user_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }


    res.status = 204;
    return;
}

void deleteRoom(httplib::Response& res, ChatRoomDB& database, const std::string room_id) {
    if (room_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing room ID"})", "application/json");
        return;
    }

    // SHOULD PROBABLY VERIFY THAT USER IS OWNER HERE

    const std::string delete_room("DELETE FROM chat.rooms WHERE room_id = ?;");

    if (!database.ModifyQuery(delete_room.c_str(), {room_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void createRoom(const httplib::Request& req, httplib::Response& res, ChatRoomDB &database) {
    if (!checkFields(req, res, {"name", "user_id"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string name = body["name"];
    const std::string user_id = body["user_id"];

    const std::string add_room("INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), '?', ?, {?}, {?}, toTimestamp(now()));");

    if (!database.ModifyQuery(add_room.c_str(), {name, user_id, user_id, user_id})) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}


#endif
