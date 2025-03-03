#ifndef CHECKFIELDS_HPP_INCLUDED
#define CHECKFIELDS_HPP_INCLUDED

#include <vector>
#include <string>

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>


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

#endif

