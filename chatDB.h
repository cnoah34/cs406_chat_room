// Compile with: g++ connect.cpp [-L <dir_containing_libscylla-cpp-driver.so> -Wl,-rpath,<dir_containing_libscylla-cpp-driver.so> -I <path_to_cassandra.h>] -lscylla-cpp-driver -o connect
// Connecting to ScyllaDB with a simple C++ program
#include <cassandra.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

void iterate_over_rows(CassFuture* future) {
    const CassResult* result = cass_future_get_result(future);
    CassIterator* iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(iterator)) {
        const CassRow* row = cass_iterator_get_row(iterator);

        if (row) {
            const char* test_value;
            size_t test_length;
            const CassValue* column1 = cass_row_get_column(row, 0);
            cass_value_get_string(column1, &test_value, &test_length);
            std::cout << "test: " << test_value << std::endl;

            cass_int32_t id_value;
            const CassValue* column2 = cass_row_get_column(row, 1);
            cass_value_get_int32(column2, &id_value);
            std::cout << "id: " << id_value << std::endl;
        }
        else {
            std::cerr << "iterate_over_rows(): error processing row from iterator" << std::endl;
        }
    }

    cass_iterator_free(iterator);
    cass_result_free(result);
}

void read_messages(CassFuture* future) {
    const CassResult* result = cass_future_get_result(future);
    CassIterator* iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(iterator)) {
        const CassRow* row = cass_iterator_get_row(iterator);

        if (row) {
            cass_int32_t room_id;
            cass_value_get_int32(cass_row_get_column(row, 0), &room_id);
            std::cout << "room_id: " << room_id << std::endl;

            cass_int64_t timestamp;
            cass_value_get_int64(cass_row_get_column(row, 1), &timestamp);
            std::time_t time_sec = timestamp / 1000;    // Convert to seconds
            std::tm* tm_time = std::gmtime(&time_sec);  // Convert to UTC
            std::cout << "timestamp: " << std::put_time(tm_time, "%Y-%m-%d %H:%M:%S") << std::endl;

            const char* content;
            size_t content_length;
            cass_value_get_string(cass_row_get_column(row, 2), &content, &content_length);
            std::cout << "content: " << content << std::endl;

            cass_int32_t message_id;
            cass_value_get_int32(cass_row_get_column(row, 3), &message_id);
            std::cout << "message_id: " << message_id << std::endl;

            cass_int32_t user_id;
            cass_value_get_int32(cass_row_get_column(row, 4), &user_id);
            std::cout << "user_id: " << user_id << std::endl;

            std::cout << std::endl;
        }
        else {
            std::cerr << "iterate_over_rows(): error processing row from iterator" << std::endl;
        }
    }

    cass_iterator_free(iterator);
    cass_result_free(result);
}

class ChatRoomDB {
    public:
        ChatRoomDB(const char* ip);
        ~ChatRoomDB();
        void Query(const char* query);

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
        std::cout << "Error connecting to ip: " << ip << std::endl;
    }
}

ChatRoomDB::~ChatRoomDB() {
    // std::cout << "Destructor" << std::endl;

    // Release the resources.
    cass_future_free(connect_future);
    cass_cluster_free(cluster);
    cass_session_free(session);
}

void ChatRoomDB::Query(const char* query) {
    CassStatement* statement = cass_statement_new(query, 0);

    CassFuture* result_future = cass_session_execute(session, statement);

    if (cass_future_error_code(result_future) == CASS_OK) {
        //iterate_over_rows(result_future);
        read_messages(result_future);
    }
    else {
        const char* message;
        size_t message_length;
        cass_future_error_message(result_future, &message, &message_length);
        fprintf(stderr, "Unable to connect: '%.*s'\n", (int)message_length, message);
    }

    return;
}

/*
int main(int argc, char* argv[]) {
    ChatRoomDB db("172.19.0.2");
    db.Query("SELECT * FROM chat.temp;");

    return 0;
}
*/
