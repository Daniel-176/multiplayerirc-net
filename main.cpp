#include "Client/Client.cpp"
#include <cstdlib>
#include <fstream>

json settings;
string currentInput;
vector<string> history;

string hexToAnsi(const std::string& hex)
{
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int g = std::stoi(hex.substr(3, 2), nullptr, 16);
    int b = std::stoi(hex.substr(5, 2), nullptr, 16);

    return "\033[38;2;" +
           std::to_string(r) + ";" +
           std::to_string(g) + ";" +
           std::to_string(b) + "m";
}

void printMessage(const string& msg) { 
    history.push_back(msg);
    cout << "\033[2J\033[H"; 
    for (const auto& msg : history)
    {
        cout << msg << '\n';
    }
    cout << "> " << currentInput << flush;
}

void loadConfig() {
    std::ifstream in("config.json");    
    if(!in) {
        json j;
        j["token"] = "please put a token there!";

        std::ofstream out("config.json");
        out << j.dump();
        out.close();

        console_log("Please input a token on config.json file, or else you'll get banned for 1 day;");
        exit(1);
    } else {
        try {
            settings = json::parse(in);
            if(settings["token"].empty()) {
                console_log("Please input a token on config.json file, or else you'll get banned for 1 day;");
                exit(1);
            }
        } catch (const std::exception& e)
        {
            cout << "Error: " << e.what() << endl;
        }
    }
}

string getToken() {
    if(!settings["token"].empty()) {
        return settings["token"];
    }   
    console_log("Token is empty!\nPlease input a token on config.json file, or else you'll get banned for 1 day;");
    exit(1);
    return "error";
}

int main() {
    loadConfig();

    Client client = Client(getToken());
    printMessage("Hi! Logging in MPP!");

    client.on("hi", [&client](auto msg) {
        client.setChannel("lobby");
        printMessage("Logged in as " + msg["u"]["name"].template get<string>());
    });

    client.on("ch", [&client](auto msg) {
        printMessage("Joined room "+client.channelName);
    });

    client.on("c", [&client](auto msg) {
        for (auto chatMessage : msg["c"]) {
            printMessage(
                hexToAnsi(chatMessage["p"]["color"].template get<string>()) +
                "[" +
                chatMessage["p"]["name"].template get<string>() +
                "] " +
                chatMessage["a"].template get<string>() + 
                "\033[0m"
            );
        }
    });

    client.on("a", [](auto msg) {
        printMessage(
            hexToAnsi(msg["p"]["color"].template get<string>()) +
            "[" +
            msg["p"]["name"].template get<string>() +
            "] " +
            msg["a"].template get<string>() + 
            "\033[0m"
        );
    });

    client.start();

    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
    }
}