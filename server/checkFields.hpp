#ifndef CHECKFIELDS_HPP_INCLUDED
#define CHECKFIELDS_HPP_INCLUDED

#include <vector>
#include <string>

#include <json.hpp>


json check_fields(const json& provided_fields, const std::vector<std::string>& required_fields) {
    json missing_fields = json::array();

    for (const auto& field : required_fields) {
        if (!provided_fields.contains(field)) {
            missing_fields.push_back(field);
        }
    }

    json response;

    if (!missing_fields.empty()) {
        response["error"] = "Missing required field(s)";
        response["missing_fields"] = missing_fields;
        response["success"] = false;
    }

    return response;
}

#endif

