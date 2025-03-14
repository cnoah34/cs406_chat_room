#ifndef CHATROOM_HPP_INCLUDED
#define CHATROOM_HPP_INCLUDED

#include <commonFunctions.hpp>

json getRoomDetails(ChatRoomDB& database, const CassUuid& room_uuid) {
    const char* query = "SELECT name, created_at FROM chat.rooms "
        "WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 1);
    cass_statement_bind_uuid(statement, 0, room_uuid);

    const json result = database.SelectQuery(statement);

    if (result.empty()) {
        return { 
            { "error", "Room does not exist" }, 
            { "code", 404 } 
        };
    }

    return result[0];
}

json getUsernames(ChatRoomDB& database, const CassUuid& room_uuid) {
    const char* query = "SELECT user_ids FROM chat.rooms WHERE room_id = ?";
    CassStatement* statement = cass_statement_new(query, 1); 
    cass_statement_bind_uuid(statement, 0, room_uuid);

    const json result = database.SelectQuery(statement);

    json users_info;

    if (!result[0].contains("user_ids")) {
        return { 
            { "error", "Could not fetch user IDs" }, 
            { "code", 404 } 
        };
    }

    for (std::string user_id : result[0]["user_ids"]) {
        CassUuid user_uuid;

        if (cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
            break;
        }

        std::string username = getUsernameFromUserId(database, user_uuid);

        if (username == "") {
            break;
        }

        users_info["users_info"].push_back({{ "user_id", user_id }, { "username", username }});
    }

    return users_info;
}

json getAdminUsernames(ChatRoomDB& database, const CassUuid& room_uuid) {
    const char* query = "SELECT admin_ids FROM chat.rooms WHERE room_id = ?";
    CassStatement* statement = cass_statement_new(query, 1); 
    cass_statement_bind_uuid(statement, 0, room_uuid);

    const json result = database.SelectQuery(statement);

    json admins_info;

    if (!result[0].contains("admin_ids")) {
        return { 
            { "error", "Could not fetch user IDs" }, 
            { "code", 404 } 
        };
    }

    for (std::string user_id : result[0]["admin_ids"]) {
        CassUuid user_uuid;

        if (cass_uuid_from_string(user_id.c_str(), &user_uuid) != CASS_OK) {
            break;
        }

        std::string username = getUsernameFromUserId(database, user_uuid);

        if (username == "") {
            break;
        }

        admins_info["admins_info"].push_back({{ "user_id", user_id }, { "username", username }});
    }

    return admins_info;
}

json getPrivilegeLevel(ChatRoomDB& database, const CassUuid& room_uuid, const CassUuid& user_uuid) {
    if (isOwner(database, room_uuid, user_uuid)) {
        return {{ "level", "Owner" }};
    }
    else if (isAdmin(database, room_uuid, user_uuid)) {
        return {{ "level", "Admin" }};
    }
    else if (isInRoom(database, room_uuid, user_uuid)) {
        return {{ "level", "User" }};
    }

    return { 
        { "error", "User does not belong to room" }, 
        { "code", 401 } 
    };
}

json removeAdmin(ChatRoomDB& database, const CassUuid& room_uuid, const CassUuid& user_uuid) {
    const char* query = "UPDATE chat.rooms SET admin_ids = admin_ids - {?} WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);

    if (!database.ModifyQuery(statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500} 
        };
    }

    return {};
}

json makeAdmin(ChatRoomDB& database, const CassUuid& room_uuid, const CassUuid& user_uuid) {
    const char* query = "UPDATE chat.rooms SET admin_ids = admin_ids + {?} WHERE room_id = ?;";

    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, user_uuid);
    cass_statement_bind_uuid(statement, 1, room_uuid);

    if (!database.ModifyQuery(statement)) {
        return { 
            { "error", "Internal server error" }, 
            { "code", 500} 
        };
    }

    return {};
}

json removeUserFromRoom(ChatRoomDB& database, const CassUuid& room_uuid, const CassUuid& user_uuid) {
    if (isOwner(database, room_uuid, user_uuid)) {
        return { 
            { "error", "Owner cannot be removed" }, 
            { "code", 400 } 
        };
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
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

json addUserToRoom(ChatRoomDB& database, CassUuid& user_uuid, CassUuid& room_uuid) {
    if (roomExists(database, room_uuid) != 1) {
        return { 
            { "error", "Room does not exist" }, 
            { "code", 400 } 
        };
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
        return { 
            { "error", "Internal server error" }, 
            { "code", 500 } 
        };
    }

    return {};
}

json deleteRoom(ChatRoomDB& database, CassUuid& room_uuid) {
    // Remove the room_id from all user's set of room_ids
    const char* get_users = "SELECT user_ids FROM chat.rooms WHERE room_id = ?";
    CassStatement* get_users_statement = cass_statement_new(get_users, 1); 
    cass_statement_bind_uuid(get_users_statement, 0, room_uuid);

    const json users = database.SelectQuery(get_users_statement);

    if (!users[0].contains("user_ids")) {
        return { 
            { "error", "Could not fetch user IDs to remove room from" }, 
            { "code", 404 } 
        };
    }

    const char* update_query = "UPDATE chat.users SET room_ids = room_ids - {?} WHERE user_id = ?;";

    for (std::string user_id : users[0]["user_ids"]) {
        CassUuid to_update_user_uuid;

        if (cass_uuid_from_string(user_id.c_str(), &to_update_user_uuid) != CASS_OK) {
            break;
        }

        CassStatement* update_statement = cass_statement_new(update_query, 2);
        cass_statement_bind_uuid(update_statement, 0, room_uuid);
        cass_statement_bind_uuid(update_statement, 1, to_update_user_uuid);

        if (!database.ModifyQuery(update_statement)) {
            return { 
                { "error", "Internal server error" }, 
                { "code", 500 } 
            };
        }
    }

    // Delete the actual room from the database
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

    svr.Get("/rooms/users/:room_id/", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        const std::string room_id = req.path_params.at("room_id");
        CassUuid room_uuid;

        if (room_id.empty() || cass_uuid_from_string(room_id.c_str(), &room_uuid ) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        json result = getUsernames(database, room_uuid);

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

    svr.Get("/rooms/admins/:room_id/", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        const std::string room_id = req.path_params.at("room_id");
        CassUuid room_uuid;

        if (room_id.empty() || cass_uuid_from_string(room_id.c_str(), &room_uuid ) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        json result = getAdminUsernames(database, room_uuid);

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

    svr.Get("/rooms/privilege/:room_id/", [&database](const httplib::Request& req, httplib::Response& res) {
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
        CassUuid room_uuid;

        if (room_id.empty() || cass_uuid_from_string(room_id.c_str(), &room_uuid ) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        json result = getPrivilegeLevel(database, room_uuid, user_uuid);

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

    // Remove admin
    svr.Patch("/rooms/demote-admin", [&database](const httplib::Request& req, httplib::Response& res) {
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

        if (!hasFields(body, { "room_id", "user_id" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];
        const std::string to_demote_user_id = body["user_id"];

        CassUuid room_uuid;
        CassUuid to_demote_user_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
            cass_uuid_from_string(to_demote_user_id.c_str(), &to_demote_user_uuid) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
            return;
        }

        if (!isOwner(database, room_uuid, user_uuid)) {
            res.status = 401;
            res.set_content(R"({"error": "Only owner can demote admins"})", "application/json");
            return;
        }
        else if (isOwner(database, room_uuid, to_demote_user_uuid)) {
            res.status = 401;
            res.set_content(R"({"error": "Owner cannot be demoted"})", "application/json");
            return;
        }

        json result = removeAdmin(database, room_uuid, to_demote_user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    // Make user admin
    svr.Patch("/rooms/promote-user", [&database](const httplib::Request& req, httplib::Response& res) {
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

        if (!hasFields(body, { "room_id", "user_id" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];
        const std::string to_promote_user_id = body["user_id"];

        CassUuid room_uuid;
        CassUuid to_promote_user_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
            cass_uuid_from_string(to_promote_user_id.c_str(), &to_promote_user_uuid) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
            return;
        }

        if (!isOwner(database, room_uuid, user_uuid)) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }
        else if (isAdmin(database, room_uuid, to_promote_user_uuid)) {
            res.status = 400;
            res.set_content(R"({"error": "User is already an admin"})", "application/json");
            return;
        }

        json result = makeAdmin(database, room_uuid, to_promote_user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    // Leave a chat room
    svr.Patch("/rooms/leave-room", [&database](const httplib::Request& req, httplib::Response& res) {
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

        if (!hasFields(body, { "room_id" })) {
            std::cout << "No room_id" << std::endl;
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];

        CassUuid room_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
            std::cout << "Bad format" << std::endl;
            res.status = 400;
            res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
            return;
        }

        json result = removeUserFromRoom(database, room_uuid, user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });

    // Remove a user from a chat room (privilege cannot exceed or match user making request) 
    svr.Patch("/rooms/remove-user", [&database](const httplib::Request& req, httplib::Response& res) {
        setCommonHeaders(res);

        std::string auth_header = req.get_header_value("Authorization");
        std::optional<CassUuid> user_uuid_opt = getUserIdFromToken(auth_header);
        if (!user_uuid_opt.has_value()) {
            res.status = 401;
            res.set_content(R"({"error": "Not authorized"})", "application/json");
            return;
        }
        CassUuid request_user_uuid = user_uuid_opt.value();

        const json body = json::parse(req.body);

        if (!hasFields(body, { "room_id", "user_id" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];
        const std::string to_remove_user_id = body["user_id"];

        CassUuid room_uuid;
        CassUuid to_remove_user_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK ||
            cass_uuid_from_string(to_remove_user_id.c_str(), &to_remove_user_uuid) != CASS_OK) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid parameter format"})", "application/json");
            return;
        }

        // Check if privilege levels work, only owner can remove admins
        if (isAdmin(database, room_uuid, to_remove_user_uuid) && !isOwner(database, room_uuid, request_user_uuid)) {
            res.status = 401;
            res.set_content(R"({"error": "Only owner can remove admins"})", "application/json");
            return;
        }

        json result = removeUserFromRoom(database, room_uuid, to_remove_user_uuid);

        if (result.contains("error")) {
            json error = {{ "error", result["error"] }};
            res.status = result["code"];
            res.set_content(error.dump(), "application/json");
        }
        else {
            res.status = 204;
        }
    });


    // Add user to room
    svr.Patch("/rooms/add-user", [&database](const httplib::Request& req, httplib::Response& res) {
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

        if (!hasFields(body, { "room_id" })) {
            res.status = 400; 
            res.set_content(R"({"error": "Missing required fields"})", "application/json");
            return;
        }

        const std::string room_id = body["room_id"];

        CassUuid room_uuid;

        if (cass_uuid_from_string(room_id.c_str(), &room_uuid) != CASS_OK) {
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
