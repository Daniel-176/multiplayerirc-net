#include "Client/Client.cpp"
#include <cstdlib>
#include <fstream>

json settings;

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
    console_log("Hi!");

    client.on("hi", [&client](auto msg) {
        client.setChannel("hello hello");
        if(msg["u"]["name"] != "C++") {
            client.setName("C++");
        }
    });

    client.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}