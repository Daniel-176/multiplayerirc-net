#include <iostream>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <string>
#include <optional>
#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <ixwebsocket/IXWebSocket.h>

using json = nlohmann::json;
using Message = nlohmann::json;

using namespace std;

class Tag {
    public:
        string text;
        string color;
};

class User {
    public:
        string name;
        string id;
        string _id;
        string color;
        string x;
        string y;
        bool afk;
        optional<Tag> tag;
};

class Client {
    public: 
        bool isConnected = false;

        Client(string TOKEN) {
            this->TOKEN = TOKEN;
            bindEventListeners();
        };

        ix::WebSocket ws;

        void emit(const std::string& event, const Message& message) {
            cout << "EMIT: " << event << endl;
            if (listeners.find(event) != listeners.end()) {
                for (auto& callback : listeners[event])
                {
                    callback(message);
                }
            }
        };

        void on(const std::string& event, std::function<void(const Message&)> callback) {
            listeners[event].push_back(callback);
        };

        void sendArray(const Message& message)
        {
            json array = json::array();

            array.push_back(message);

            ws.send(array.dump());
        };

        void bindEventListeners() {
            on("b", [this](auto msg) {
                cout << "<b>";
            });
        };

        void start() {
            if (isConnected == true) return;

            ws.setUrl("wss://backend.multiplayerpiano.net/");

            ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message)
                {
                    cout << "RAW: " << msg->str << endl;
                    try {
                        auto messages = json::parse(msg->str);

                        for(auto& message : messages)
                        {
                            if(message.contains("m"))
                            {
                                emit(message["m"], message);
                            }
                        }
                        cout << msg->str;
                    } catch (const json::parse_error& e) {
                        cout << "JSON inválido: " << e.what();
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Open)
                {
                    emit("connected", {
                        {"ws-state", ws.getReadyState()}
                    });
                    isConnected = true;

                    sendArray({
                        {"m", "hi"},
                        {"token", TOKEN}
                    });
                }
                else if (msg->type == ix::WebSocketMessageType::Error)
                {
                    // Maybe SSL is not configured properly
                    std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
                }
                else if (msg->type == ix::WebSocketMessageType::Close)
                {
                    emit("disconnected", {});
                    
                    //debug
                    cout << "CLOSE CODE: "
                        << msg->closeInfo.code
                        << " REASON: "
                        << msg->closeInfo.reason
                        << endl;

                    isConnected = false;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    ws.connect(5);
                }
            });


            ws.connect(5);
        }
    
    private:
        map<string, vector<function<void(const Message&)>>> listeners;
        string TOKEN;
};