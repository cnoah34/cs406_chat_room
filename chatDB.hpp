#ifndef CHATDB_H_INCLUDED
#define CHATDB_H_INCLUDED

// Compile with: g++ connect.cpp [-L <dir_containing_libscylla-cpp-driver.so> -Wl,-rpath,<dir_containing_libscylla-cpp-driver.so> -I <path_to_cassandra.h>] -lscylla-cpp-driver -o connect
#include <cassandra.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>

#include <json.hpp>
using json = nlohmann::json;

std::set<int> read_set(const CassValue* set) {
    CassIterator* iterator = cass_iterator_from_collection(set);

    std::set<int> result;

    while (cass_iterator_next(iterator)) {
        const CassValue* iter_value = cass_iterator_get_value(iterator);
        cass_int32_t value;

        if (cass_value_get_int32(iter_value, &value) == CASS_OK) {
            result.insert(value);
        }
    }

    cass_iterator_free(iterator);

    return result;
}

std::string read_timestamp(cass_int64_t timestamp) {
    std::time_t time_sec = timestamp / 1000;    // Convert to seconds
    std::tm* tm_time = std::gmtime(&time_sec);  // Convert to UTC

    // ISO 8601 format "YYYY-MM-DDTHH:MM:SSZ"
    const char* format = "%Y-%m-%dT%H:%M:%SZ";
    std::stringstream ss;

    ss << std::put_time(tm_time, format);

    return ss.str();
}

json dynamic_read(CassFuture* future) {
    json large_json;

    const CassResult* result = cass_future_get_result(future);

    if (cass_result_row_count(result) == 0) {
        std::cout << "Query returned no rows" << std::endl;
        cass_result_free(result);
        return large_json;
    }

    CassIterator* row_iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(row_iterator)) {
        json json_object;
        
        const CassRow* row = cass_iterator_get_row(row_iterator);

        size_t column_count = cass_result_column_count(result);

        for (size_t i = 0; i < column_count; i++) {
            // Get column metadata
            const CassValue* column_value = cass_row_get_column(row, i); 

            // Column name
            const char* col_name;
            size_t col_name_length;
            cass_result_column_name(result, i, &col_name, &col_name_length);
            std::string column_name(col_name, col_name_length);

            // Column type
            CassValueType column_type = cass_value_type(column_value);

            switch (column_type) {
                case CASS_VALUE_TYPE_INT:
                    cass_int32_t int_value;
                    cass_value_get_int32(column_value, &int_value);
                    json_object[column_name] = int_value;
                    break;
                case CASS_VALUE_TYPE_BIGINT:
                    cass_int64_t bigint_value;
                    cass_value_get_int64(column_value, &bigint_value);
                    json_object[column_name] = bigint_value;
                    break;
                case CASS_VALUE_TYPE_TIMESTAMP:
                    cass_int64_t timestamp_value;
                    cass_value_get_int64(column_value, &timestamp_value);
                    json_object[column_name] = read_timestamp(timestamp_value);
                    break;
                case CASS_VALUE_TYPE_VARCHAR: {
                    const char* text_value;
                    size_t text_length;
                    cass_value_get_string(column_value, &text_value, &text_length);
                    json_object[column_name] = std::string(text_value, text_length);
                    break;
                }
                case CASS_VALUE_TYPE_SET:
                    json_object[column_name] = read_set(column_value);
                    break;
                default:
                    break;
            }
        }

        large_json.push_back(json_object);
    }

    cass_iterator_free(row_iterator);
    cass_result_free(result);

    return large_json;
}

class ChatRoomDB {
    public:
        ChatRoomDB(const char* ip);
        ~ChatRoomDB();
        json SelectQuery(const char* query);

    private:
        // Allocate the objects that represent cluster and session. Remember to free them once no longer needed!
        CassCluster* cluster = cass_cluster_new();
        CassSession* session = cass_session_new();
        CassFuture* connect_future = nullptr;
};

ChatRoomDB::ChatRoomDB(const char* ip) : connect_future(nullptr) {
    // std::cout << "Constructor" << std::endl;

    // Add the contact points. These can be either IPs or domain names.
    // You can specify more than one, comma-separated, but you don’t have to - driver will discover other nodes by itself. You should do it if you expect some of your contact points to be down.
    cass_cluster_set_contact_points(cluster, ip);

    // Connect. `cass_session_connect` returns a pointer to "future"
    connect_future = cass_session_connect(session, cluster);

    // `cass_future_error_code` will block until connected or refused.
    if (cass_future_error_code(connect_future) == CASS_OK) {
        std::cout << "Connected" << std::endl;
    }
    else {
        std::cerr << "Error connecting to database at: " << ip << std::endl;
    }
}

ChatRoomDB::~ChatRoomDB() {
    // std::cout << "Destructor" << std::endl;

    // Release the resources.
    cass_future_free(connect_future);
    cass_cluster_free(cluster);
    cass_session_free(session);
}

json ChatRoomDB::SelectQuery(const char* query) {
    CassStatement* statement = cass_statement_new(query, 0);

    CassFuture* result_future = cass_session_execute(session, statement);

    json json_result;

    if (cass_future_error_code(result_future) == CASS_OK) {
        json_result = dynamic_read(result_future);
    }
    else {
        const char* message;
        size_t message_length;
        cass_future_error_message(result_future, &message, &message_length);
        fprintf(stderr, "Query error: '%.*s'\n", (int)message_length, message);
    }

    cass_statement_free(statement);
    cass_future_free(result_future);

    return json_result;
}

#endif

