#include <iostream>
#include <unordered_set>
#include "crow.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class ParseCoordinatesException : public std::exception {
public:
    ParseCoordinatesException() noexcept = default;
    ~ParseCoordinatesException() = default;

    virtual const char *what() const noexcept override {
        return "Some error occurred while trying to parse coordinates.";
    }
};

int main() {
    auto app = crow::SimpleApp();

    CROW_ROUTE(app, "/host")
    ([](crow::response &res) {
        res.set_static_file_info("public/host.html");
        res.end();
    });

    CROW_ROUTE(app, "/player")
    ([](crow::response &res) {
        res.set_static_file_info("public/player.html");
        res.end();
    });

    std::pmr::unordered_set<crow::websocket::connection*> clients;

    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([&](crow::websocket::connection& conn) {
            clients.insert(&conn);
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            clients.erase(&conn);
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            for (auto* client : clients) {
                if (client != &conn) {
                    client->send_text(data);
                }
            }
        });

    app.port(4040).multithreaded().run();

    return 0;
}