// Compile with: g++ connect.cpp [-L <dir_containing_libscylla-cpp-driver.so> -Wl,-rpath,<dir_containing_libscylla-cpp-driver.so> -I <path_to_cassandra.h>] -lscylla-cpp-driver -o connect
// Connecting to ScyllaDB with a simple C++ program
#include <cassandra.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <vector>

struct Message {
    cass_int32_t room_id;
    cass_int64_t created_at;
    const char* content;
    cass_int32_t message_id;
    cass_int32_t user_id;
};

struct User {
    cass_int32_t user_id;
    cass_int64_t created_at;
    const char* password;
    std::vector<int> room_ids;
    const char* username;
};

struct Room {
    cass_int32_t room_id;
    std::vector<int> admin_ids;
    cass_int64_t created_at;
    const char* name;
    std::vector<int> user_ids;
};


void dynamic_read(const CassResult* result) {
    CassIterator* row_iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(row_iterator)) {
        const CassRow* row = cass_iterator_get_row(row_iterator);

    }

    return;
}

std::vector<int> read_set(const CassValue* set) {
    CassIterator* iterator = cass_iterator_from_collection(set);

    std::vector<int> vec;

    while (cass_iterator_next(iterator)) {
        const CassValue* iter_value = cass_iterator_get_value(iterator);
        cass_int32_t value;

        if (cass_value_get_int32(iter_value, &value) == CASS_OK) {
            vec.push_back(value);
        }
    }

    return vec;
}

void print_message(Message message) {
    std::cout << std::endl << "room_id: " << message.room_id << std::endl;

    std::time_t time_sec = message.created_at / 1000;   // Convert to seconds
    std::tm* tm_time = std::gmtime(&time_sec);          // Convert to UTC
    std::cout << "timestamp: " << std::put_time(tm_time, "%Y-%m-%d %H:%M:%S") << std::endl;

    std::cout << "content: " << message.content << std::endl;

    std::cout << "message_id: " << message.message_id << std::endl;

    std::cout << "user_id: " << message.user_id << std::endl << std::endl;

    return;
}

void read_messages(CassFuture* future) {
    const CassResult* result = cass_future_get_result(future);
    CassIterator* iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(iterator)) {
        const CassRow* row = cass_iterator_get_row(iterator);

        if (row) {
            Message message;


            cass_value_get_int32(cass_row_get_column_by_name(row, "room_id"), &message.room_id);

            cass_value_get_int64(cass_row_get_column_by_name(row, "created_at"), &message.created_at);

            size_t content_length;
            cass_value_get_string(cass_row_get_column_by_name(row, "content"), &message.content, &content_length);

            cass_value_get_int32(cass_row_get_column_by_name(row, "message_id"), &message.message_id);

            cass_value_get_int32(cass_row_get_column_by_name(row, "user_id"), &message.user_id);


            print_message(message);
        }
        else {
            std::cerr << "iterate_over_rows(): error processing row from iterator" << std::endl;
        }
    }

    cass_iterator_free(iterator);
    cass_result_free(result);
}

void print_user(User user) {
    std::cout << std::endl << "user_id: " << user.user_id << std::endl;

    std::time_t time_sec = user.created_at / 1000;   // Convert to seconds
    std::tm* tm_time = std::gmtime(&time_sec);          // Convert to UTC
    std::cout << "timestamp: " << std::put_time(tm_time, "%Y-%m-%d %H:%M:%S") << std::endl;

    std::cout << "password: " << user.password << std::endl;

    std::cout << "room_ids: [";
    for (size_t i = 0; i < user.room_ids.size(); i++) {
        std::cout << user.room_ids[i];

        if (i != user.room_ids.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    std::cout << "username: " << user.username << std::endl;

    return;
}

void read_users(CassFuture* future) {
    const CassResult* result = cass_future_get_result(future);
    CassIterator* iterator = cass_iterator_from_result(result);

    while (cass_iterator_next(iterator)) {
        const CassRow* row = cass_iterator_get_row(iterator);

        if (row) {
            User user;


            cass_value_get_int32(cass_row_get_column_by_name(row, "user_id"), &user.user_id);
            cass_value_get_int64(cass_row_get_column_by_name(row, "created_at"), &user.created_at);

            size_t password_length;
            cass_value_get_string(cass_row_get_column_by_name(row, "password"), &user.password, &password_length);

            user.room_ids = read_set(cass_row_get_column_by_name(row, "room_ids"));

            size_t username_length;
            cass_value_get_string(cass_row_get_column_by_name(row, "username"), &user.username, &username_length);


            print_user(user);
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
        //read_messages(result_future);
        read_users(result_future);
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
