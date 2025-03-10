#ifndef WEBSOCKETMANAGER_HPP_INCLUDED 
#define WEBSOCKETMANAGER_HPP_INCLUDED 

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <uwebsockets/App.h>


struct UserData {
    std::string user_id;
};


class WebSocketManager {
    public: 
        // A map of room_ids to a set of websocket connections to users (I cannot lie, this is a bit much)
        std::unordered_map<std::string, std::unordered_set<uWS::WebSocket<false, true, UserData>*>> rooms;

        void addUserToRoom(const std::string& room_id, uWS::WebSocket<false, true, UserData>* ws) {
            rooms[room_id].insert(ws);
            std::cout << "User added to room: " << room_id << std::endl;
        }

        void removeUserFromRoom(const std::string& room_id, uWS::WebSocket<false, true, UserData>* ws) {
            auto it = rooms.find(room_id);

            if (it != rooms.end()) {
                it->second.erase(ws);

                std::cout << "User removed from room: " << room_id << std::endl;

                if (it->second.empty()) {
                    rooms.erase(room_id);
                    std::cout << "Room erased: " << room_id << std::endl;
                }
            }
        }
    
        void broadcastToRoom(const std::string& room_id, const std::string& message) {
            auto it = rooms.find(room_id);

            if (it != rooms.end()) {
                for (const auto& ws : it->second) {
                    ws->send(message);
                }
            }
        }
};


#endif
