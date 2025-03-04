#ifndef CHECKFIELDS_HPP_INCLUDED
#define CHECKFIELDS_HPP_INCLUDED

#include <vector>
#include <string>
#include <optional>
#include <jwt-cpp/jwt.h>

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>

#include <chatDB.hpp>

using json = nlohmann::json;


void setCommonHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.set_header("Access-Control-Allow-Credentials", "true");

    return;
}

bool checkFields(const httplib::Request& req, httplib::Response& res, const std::vector<std::string>& required_fields) {
    json requestBody = json::parse(req.body);
    json missing_fields = json::array();

    for (const auto& field : required_fields) {
        if (!requestBody.contains(field)) {
            missing_fields.push_back(field);
        }
    }

    if (!missing_fields.empty()) {
        json responseBody;
        responseBody["error"] = "Missing required field(s)";
        responseBody["missing_fields"] = missing_fields;

        res.status = 400;
        res.set_content(responseBody.dump(), "application/json");

        return false;
    }

    return true;
}

std::optional<CassUuid> getUserIdFromToken(std::string& authHeader) {
    if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
        return std::nullopt;
    }

    std::string token = authHeader.substr(7);

    try {
        const char* secret_cstr = std::getenv("JWT_SECRET");
        if (!secret_cstr) {
            std::cerr << "JWT secret not found in environment variables" << std::endl;
            return std::nullopt;
        }

        std::string secret(secret_cstr);
        if (secret.empty()) {
            return std::nullopt;
        }

        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer("chat_rooms");

        verifier.verify(decoded);

        if (decoded.has_payload_claim("user_id")) {
            std::string user_id_str = decoded.get_payload_claim("user_id").as_string();
            CassUuid user_uuid;

            CassError rc = cass_uuid_from_string(user_id_str.c_str(), &user_uuid);

            if (rc != CASS_OK) {
                std::cerr << "Invalid UUID format in token" << std::endl;
                return std::nullopt;
            }

            return user_uuid;
        }
    }
    catch (const std::exception& e) {
        return std::nullopt;
    }

    return std::nullopt;
}

bool userInRoom(ChatRoomDB& database, CassUuid user_uuid, CassUuid room_uuid) {
    const char* query = "SELECT COUNT(*) FROM chat.rooms WHERE room_id = ? AND user_ids CONTAINS ?;";
    CassStatement* statement = cass_statement_new(query, 2);
    cass_statement_bind_uuid(statement, 0, room_uuid);
    cass_statement_bind_uuid(statement, 1, user_uuid);

    json result = database.SelectQuery(statement);

    if (result.empty() || !result[0].contains("count") || result[0]["count"] == 0) {
        return false;
    }
    else if (result[0]["count"] == 1) {
        return true;
    }

    return false;
}


#endif

