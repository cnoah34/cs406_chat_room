#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include <uwebsockets/App.h>

// Custom
#include <chatDB.hpp>
#include <users.hpp>
#include <messages.hpp>
#include <rooms.hpp>
#include <webSocketManager.hpp>

using json = nlohmann::json;


int main() {
    const char* rest_port_str = std::getenv("REST_PORT");
    if (rest_port_str == nullptr) {
        std::cerr << "REST_PORT environment variable not set" << std::endl;
        return 1;
    }
    int rest_port = std::stoi(rest_port_str);

    const char* websocket_port_str = std::getenv("WEBSOCKET_PORT");
    if (websocket_port_str == nullptr) {
        std::cerr << "WEBSOCKET_PORT environment variable not set" << std::endl;
        return 1;
    }
    int websocket_port = std::stoi(websocket_port_str);

    const char* database_ip = std::getenv("DATABASE_IP");
    if (database_ip == nullptr) {
        std::cerr << "DATABASE_IP environment variable not set" << std::endl;
        return 1;
    }

    ChatRoomDB database(database_ip);
    WebSocketManager ws_manager;

    std::thread rest_server_thread([&database, &ws_manager, &rest_port]() {
        httplib::SSLServer svr("./cert.pem", "./key.pem");

        if (!svr.is_valid()) {
            std::cout << "Fail" << std::endl;
        }

        svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
            setCommonHeaders(res);
            res.status = 204;
        });

        defineUserMethods(svr, database);
        defineMessageMethods(svr, database);
        defineRoomMethods(svr, database);
        definePostMessage(svr, database, ws_manager);

        std::cout << "REST server listening on port: " << rest_port << std::endl;
        svr.listen("0.0.0.0", rest_port);
    });

    std::thread websocket_server_thread([&ws_manager, websocket_port]() {
        uWS::SSLApp ssl_app({
            .key_file_name = "./key.pem",
            .cert_file_name = "./cert.pem"
        });

        uWS::TemplatedApp<true>::WebSocketBehavior<UserData> ws_behavior;

        ws_behavior.close = [&](uWS::WebSocket<true, true, UserData>* ws, int code, std::string_view message) {
            //std::cout << "Websocket closed" << std::endl;
            UserData* user_data = ws->getUserData();
            if (!user_data) {
                return;
            }
            
            ws_manager.removeUserFromRoom(user_data->room_id, ws);
        };

        ws_behavior.message = [&](uWS::WebSocket<true, true, UserData>* ws, std::string_view message, uWS::OpCode op_code) {
            //std::cout << "Received message: "<< message << std::endl;

            json body = json::parse(message);

            if (body.contains("room_id")) {
                UserData* user_data = ws->getUserData();
                if (!user_data) {
                    return;
                }

                if (!user_data->room_id.empty()) {
                    ws_manager.removeUserFromRoom(user_data->room_id, ws);
                }

                user_data->room_id = body["room_id"];

                ws_manager.addUserToRoom(body["room_id"], ws);
            }
            else if (body.contains("message") && body["message"].contains("room_id")) {
                ws_manager.broadcastToRoom(body["message"]["room_id"], body["message"]);
            }
        };

        ssl_app.ws<UserData>("/", std::move(ws_behavior));

        ssl_app.listen(websocket_port, [&](auto* listen_socket) {
            if (listen_socket) {
                std::cout << "Websocket server listening on port: " << websocket_port << std::endl;
            }
            else {
                std::cerr << "Failed to start websocket server on port: " << websocket_port << std::endl;
            }
        });

        ssl_app.run();
    });

    rest_server_thread.join();
    websocket_server_thread.join();

    return 0;
}
