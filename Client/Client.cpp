#include <cstddef>
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
#include "Utils.cpp"

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
        thread pingThread;
        float serverTime;
        json channel;
        json ppl;
        json user;

        Client(string TOKEN) {
            this->TOKEN = TOKEN;
            bindEventListeners();
        };

        ix::WebSocket ws;

        void emit(const std::string& event, const Message& message) {
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

            on("hi", [this](auto msg) {
                serverTime = msg["t"];
                ppl = msg["ppl"];

                // yeah i completely ignored the user class.
                user = msg["u"];
            });

            on("ch", [this](auto msg) {
                channel = msg["ch"];
                ppl = msg["ppl"];
            });

            on("p", [this](auto part) {
                if(!ppl.contains(part["id"])) {
                    ppl[part["id"]] = part;
                    emit("participant added", part);
                } else {
                    ppl[part["id"]] = part;
                    emit("participant update", ppl[part["id"]]);
                }
            });

            on("bye", [this](auto msg) {
                ppl = {};
                user = {};
                serverTime = 0;
            });
        };

        long long DateNow() {
            using namespace std::chrono;
            return duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
            ).count();
        };

        void sendPing() {
            sendArray({
                {"m", "t"},
                {"e", DateNow()}
            });
        };

        void setChannel(string channel) {
            sendArray({
                {"m", "ch"},
                {"_id", channel}
            });
        };

        // Client stuff

        void setName(string name) {
            sendArray({
                {"m", "userset"},
                {"set", {
                    {"name", name}
                }}
            });
        };

        void setColor(string color) {
            sendArray({
                {"m", "userset"},
                {"set", {
                    {"color", color}
                }}
            });
        };

        void userset(json set) {
            sendArray({
                {"m", "userset"},
                {"set", set}
            });
        };

        void say(string message) {
            sendArray({
                {"m", "a"},
                {"message", message}
            });
        };

        void moveMouse(float x, float y) {
            sendArray({
                {"m", "m"},
                {"x", x},
                {"y", y}
            });
        };

        // Room Management

        void kickBan(string _id, float ms) {
            sendArray({
                {"m", "kickban"},
                {"_id", _id},
                {"ms", ms}
            });
        };

        void chown(string _id) {
            sendArray({
                {"m", "chown"},
                {"id", _id}
            });
        };

        void start() {
            if (isConnected == true) return;

            ws.setUrl("wss://backend.multiplayerpiano.net/");
            
            ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
                if (msg->type == ix::WebSocketMessageType::Message)
                {
                    try
                    {
                        auto messages = json::parse(msg->str);

                        for (auto& message : messages)
                        {
                            if (message.contains("m"))
                            {
                                emit(message["m"], message);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        cout << "Error: " << e.what() << endl;
                        cout << "msg: " << msg->str << endl;
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Open)
                {
                    isConnected = true;

                    sendArray({
                        {"m", "hi"},
                        {"token", TOKEN}
                    });

                    pingThread = std::thread([this]() {
                        while (isConnected)
                        {
                            sendPing();
                            std::this_thread::sleep_for(std::chrono::seconds(20));
                        }
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

                    if (pingThread.joinable())
                    {
                        pingThread.join();
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    ws.connect(5);
                }
            });


            ws.connect(5);
            ws.start();
        };
    private:
        map<string, vector<function<void(const Message&)>>> listeners;
        string TOKEN;
};