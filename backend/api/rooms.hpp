#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

#include <commonFunctions.hpp>


void getRoomDetails(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    std::string authHeader = req.get_header_value("Authorization");

    std::optional<CassUuid> userUuidOpt = getUserIdFromToken(authHeader);
    if (!userUuidOpt.has_value()) {
        res.status = 401;
        res.set_content(R"({"error": "Not authorized"})", "application/json");
        return;
    }

    CassUuid user_uuid = userUuidOpt.value();

    const std::string room_id = req.path_params.at("room_id");

    if (room_id.empty()) {
        res.status = 400;
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

    CassUuid room_uuid;

    if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
        res.status = 400;
        res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
        return;
    }

    const char* query = "SELECT name, created_at FROM chat.rooms "
        "WHERE user_ids CONTAINS ? AND room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        res.status = 404;
        res.set_content(R"({"error": "No room found"})", "application/json");
        return;
    }

    res.status = 200;
    res.set_content(result[0].dump(), "application/json");
    return;
}

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
    std::string authHeader = req.get_header_value("Authorization");

    std::optional<CassUuid> userUuidOpt = getUserIdFromToken(authHeader);
    if (!userUuidOpt.has_value()) {
        res.status = 401;
        res.set_content(R"({"error": "Not authorized"})", "application/json");
        return;
    }

    CassUuid user_uuid = userUuidOpt.value();

    if (!checkFields(req, res, {"name"})) {
        return;
    }

    const json body = json::parse(req.body);
    const std::string name = body["name"];

    CassCollection* admin_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(admin_set, user_uuid);
    CassCollection* user_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(user_set, user_uuid);

    CassUuid room_uuid;
    CassUuidGen* uuid_gen = cass_uuid_gen_new();
    cass_uuid_gen_random(uuid_gen, &room_uuid);
    cass_uuid_gen_free(uuid_gen);

    const char* insert_room = "INSERT INTO chat.rooms (room_id, name, owner_id, admin_ids, user_ids, created_at) " 
            "VALUES (?, ?, ?, ?, ?, toTimestamp(now()));";

    CassStatement* statement = cass_statement_new(insert_room, 5);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_string(statement, 1, name.c_str());
    cass_statement_bind_uuid(statement, 2, user_uuid);
    cass_statement_bind_collection(statement, 3, admin_set);
    cass_statement_bind_collection(statement, 4, user_set);

    if (!database.ModifyQuery(statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    const char* add_room_to_user = "UPDATE chat.users SET room_ids = room_ids + {?} WHERE user_id = ?;";
    CassStatement* add_to_user_statement = cass_statement_new(add_room_to_user, 2);
    cass_statement_bind_uuid(add_to_user_statement, 0, room_uuid);
    cass_statement_bind_uuid(add_to_user_statement, 1, user_uuid);

    if (!database.ModifyQuery(add_to_user_statement)) {
        res.status = 500;
        res.set_content(R"({"error": "Internal server error"})", "application/json");
        return;
    }

    res.status = 204;
    return;
}

void defineRoomMethods(httplib::Server& svr, ChatRoomDB& database) {
    // Get details of a room
    svr.Get("/rooms/:room_id/", [&database](const httplib::Request& req, httplib::Response& res) {
        getRoomDetails(req, res, database);
        setCommonHeaders(res);
    });

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
