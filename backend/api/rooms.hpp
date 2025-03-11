#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

#include <commonFunctions.hpp>

json getRoomDetails(ChatRoomDB& database, const CassUuid& room_id) {
    const char* query = "SELECT name, created_at FROM chat.rooms "
        "WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, room_id);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        return { 
            { "error", "Room does not exist" }, 
            { "code", 404 } 
        };
    }

    return result[0];
}

/*
void removeAdmin(const httplib::Request& req, httplib::Response& res, ChatRoomDB& database) {
    const json body = json::parse(req.body);

    if (!hasFields(body, { "room_id", "user_id" })) {
        res.status = 400; 
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

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
    const json body = json::parse(req.body);

    if (!hasFields(body, { "room_id", "user_id" })) {
        res.status = 400; 
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

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
    const json body = json::parse(req.body);

    if (!hasFields(body, { "room_id", "user_id" })) {
        res.status = 400; 
        res.set_content(R"({"error": "Missing required fields"})", "application/json");
        return;
    }

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
*/

json addUserToRoom(ChatRoomDB& database, CassUuid& user_uuid, CassUuid& room_uuid) {
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
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

json deleteRoom(ChatRoomDB& database, CassUuid& room_uuid) {
    const char* query = "DELETE FROM chat.rooms WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, room_uuid);

    if (!database.ModifyQuery(statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

// TODO: Change to transaction?
json createRoom(ChatRoomDB &database, const CassUuid& user_uuid, const std::string& name) {
    // Add the owner to the initial set of admins and users
    CassCollection* admin_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(admin_set, user_uuid);
    CassCollection* user_set = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
    cass_collection_append_uuid(user_set, user_uuid);

    // Generate a uuid for room_id
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
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    const char* add_room_to_user = "UPDATE chat.users SET room_ids = room_ids + {?} WHERE user_id = ?;";
    CassStatement* add_to_user_statement = cass_statement_new(add_room_to_user, 2);
    cass_statement_bind_uuid(add_to_user_statement, 0, room_uuid);
    cass_statement_bind_uuid(add_to_user_statement, 1, user_uuid);

    if (!database.ModifyQuery(add_to_user_statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

void defineRoomMethods(httplib::Server& svr, ChatRoomDB& database) {
    // Get details of a room
    svr.Get("/rooms/:room_id/", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        const std::string room_id = req.path_params.at("room_id");
        CassUuid room_uuid;

        if (room_id.empty() || cass_uuid_from_string(room_id.c_str(), &room_uuid ) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        json result = getRoomDetails(database, room_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 200;
            res.set_content(result.dump(), "application/json");
        }
    });

    /*
    // Remove admin
    svr.Patch("/rooms/remove_admin", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);
        removeAdmin(req, res, database);
    });

    // Make user admin
    svr.Patch("/rooms/make_admin", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);
        makeUserAdmin(req, res, database);
    });

    // Remove user from room
    svr.Patch("/rooms/remove_user", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);
        removeUserFromRoom(req, res, database);
    });
    */

    // Add user to room
    svr.Patch("/rooms/add_user", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        const json body = json::parse(req.body);

        if (!hasFields(body, { "room_id", "user_id" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

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

        json result = addUserToRoom(database, user_uuid, room_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    // Delete room
    svr.Delete("/rooms/:room_id", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);
        
        std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

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

        if (!isOwner(database, room_uuid, user_uuid)) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        json result = deleteRoom(database, room_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    // Create room
    svr.Post("/rooms", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        std::string auth_header = req.get_header_value("Authorization");

        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }

        CassUuid user_uuid = user_uuid_opt.value();

        const json body = json::parse(req.body);

        if (!hasFields(body, { "name" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string name = body["name"];

        if (name == "") {
            res.status = 400; 
            res.set_content(R"({"error": "Invalid room name"})", "application/json");
            return;
        }

        json result = createRoom(database, user_uuid, name);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    return;
}


#endif
