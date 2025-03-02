#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

// Custom
#include <chatDB.hpp>
#include <commonFunctions.hpp>


void removeAdmin(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);
    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    CassUuid room_uuid;
    CassUuid user_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
        cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = "UPDATE chat.rooms SET admin_ids = admin_ids - {?} WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void makeUserAdmin(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    if (!checkFields(req, res, {"room_id", "user_id"})) {
        return;
    }

    json body = json::parse(req.body);
    const std::string room_id = body["room_id"];
    const std::string user_id = body["user_id"];

    CassUuid room_uuid;
    CassUuid user_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
        cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = "UPDATE chat.rooms SET admin_ids = admin_ids + {?} WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);

    if (!database.ModifyQuery(statement)) {
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

    CassUuid room_uuid;
    CassUuid user_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
        cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids - {?} WHERE room_id = ?; "
            "UPDATE chat.rooms SET admin_ids = admin_ids - {?} WHERE room_id = ?; "
            "UPDATE chat.users SET room_ids = room_ids - {?} WHERE user_id = ?; "
            "APPLY BATCH;";

    CassStatement* statement = cass_statement_new(query, 6);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);
    cass_statement_bind_uuid(statement, 2, user_uuid);
    cass_statement_bind_uuid(statement, 3, room_uuid);
    cass_statement_bind_uuid(statement, 4, room_uuid);
    cass_statement_bind_uuid(statement, 5, user_uuid);

    if (!database.ModifyQuery(statement)) {
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

    CassUuid room_uuid;
    CassUuid user_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
        cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = 
            "BEGIN BATCH "
            "UPDATE chat.rooms SET user_ids = user_ids + {?} WHERE room_id = ?; "
            "UPDATE chat.users SET room_ids = room_ids + {?} WHERE user_id = ?; "
            "APPLY BATCH;";

    CassStatement* statement = cass_statement_new(query, 4);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);
    cass_statement_bind_uuid(statement, 2, room_uuid);
    cass_statement_bind_uuid(statement, 3, user_uuid);

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }


    res.status = 204;
    return;
}

void deleteRoom(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    const std::string room_id = req.path_params.at("room_id");
    
    if (room_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    // SHOULD PROBABLY VERIFY THAT USER IS OWNER

    CassUuid room_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = "DELETE FROM chat.rooms WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, room_uuid);

    if (!database.ModifyQuery(statement)) {
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
    const std::string user_id = body["user_id"];
    const std::string name = body["name"];

    CassUuid user_uuid;

    if (cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    CassCollection* admin_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(admin_set, user_uuid);
    CassCollection* user_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(user_set, user_uuid);

    const char* query = "INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (uuid(), ?, ?, ?, ?, toTimestamp(now()));";

    CassStatement* statement = cass_statement_new(query, 4);
    cass_statement_bind_string(statement, 0, name.c_str());
    cass_statement_bind_uuid(statement, 1, user_uuid);
    cass_statement_bind_collection(statement, 2, admin_set);
    cass_statement_bind_collection(statement, 3, user_set);

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void defineRoomMethods(httplib::Server& svr, ChatRoomDB& database) {
    // Remove admin
    svr.Patch("/rooms/remove_admin", [&database](const httplib::Request& req, httplib::Response& res) {
        removeAdmin(req, res, database);
        setCommonHeaders(res);
    });

    // Make user admin
    svr.Patch("/rooms/make_admin", [&database](const httplib::Request& req, httplib::Response& res) {
        makeUserAdmin(req, res, database);
        setCommonHeaders(res);
    });

    // Remove user from room
    svr.Patch("/rooms/remove_user", [&database](const httplib::Request& req, httplib::Response& res) {
        removeUserFromRoom(req, res, database);
        setCommonHeaders(res);
    });

    // Add user to room
    svr.Patch("/rooms/add_user", [&database](const httplib::Request& req, httplib::Response& res) {
        addUserToRoom(req, res, database);
        setCommonHeaders(res);
    });

    // Delete room
    svr.Delete("/rooms/:room_id", [&database](const httplib::Request& req, httplib::Response& res) {
        deleteRoom(req, res, database);
        setCommonHeaders(res);
    });

    // Create room
    svr.Post("/rooms", [&database](const httplib::Request& req, httplib::Response& res) {
        createRoom(req, res, database);
        setCommonHeaders(res);
    });
    
    return;
}


#endif
