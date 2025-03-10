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
        httplib::Server svr;

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
        uWS::App().ws<UserData>("/ws", {
            .open = [&](uWS::WebSocket<false, true, UserData>* ws) {
                std::cout << "New websocket connection" << std::endl;
            },
            .message = [&](uWS::WebSocket<false, true, UserData>* ws, std::string_view message, uWS::OpCode op_code) {
                std::cout << "Received message: "<< message << std::endl;

                json body = json::parse(message);

                if (body.contains("room_id")) {
                    ws_manager.addUserToRoom(body["room_id"], ws);
                }
                else if (body.contains("message") && body["message"].contains("room_id")) {
                    ws_manager.broadcastToRoom(body["message"]["room_id"], body["message"]);
                }
            },
            .close = [&](uWS::WebSocket<false, true, UserData>* ws, int code, std::string_view message) {
                std::cout << "Websocket closed" << std::endl;
                ws_manager.removeUserFromRoom("default", ws);
            }
        }).listen(websocket_port, [&](auto* token) {
            if (token) {
                std::cout << "Websocket server listening on port: " << websocket_port << std::endl;
            }
            else {
                std::cerr << "Failed to start websocket server on port: " << websocket_port << std::endl;
            }
        }).run();
    });

    rest_server_thread.join();
    websocket_server_thread.join();

    return 0;
}
