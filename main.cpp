#include "Client/Client.cpp"
#include <cstddef>
#include <cstdlib>
#include <fstream>

json settings;
string currentInput;
string desiredChannel;
vector<string> history;

const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string DIM    = "\033[2m";
const string ITALIC = "\033[3m";
const string UNDER  = "\033[4m";

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

// The stylizing won't work since it's not lively changed, kinda lazy to do a redraw function.
void drawPrompt() {
    cout << "> ";
    if(!currentInput.empty() && currentInput[0] == '/') {
        size_t space = currentInput.find(" ");

        if(space == string::npos) {
            cout << "\033[1;34m"
                 << currentInput
                 << "\033[0m";
        } else {
            cout << "\033[1;34m"
                 << currentInput.substr(0, space)
                 << "\033[0m"
                 << currentInput.substr(space);
        }
    } else {
        cout << currentInput;
    }

    cout << flush;
}

void printMessage(const string& msg) { 
    history.push_back(msg);
    cout << "\033[2J\033[H"; 
    for (const auto& msg : history)
    {
        cout << msg << '\n';
    }
    drawPrompt();
}

void clear() {
    history.clear();
    cout << "\033[2J\033[H"; 
    drawPrompt();
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

int main(int argc, char *argv[]) {
    loadConfig();

    for(int i = 0; i < argc; i++) {
        string arg = argv[i];

        if(arg == "--help" || arg == "-h" || arg == "help") {
            console_log("Usage: mpp [CHANNEL]");
            console_log("\nIn-Client commands:");
            console_log("\n\n/nick [USERNAME]   -   Sets your name in MPP.");
            console_log("\n/join [CHANNEL]   -   Join a channel in MPP.");
            console_log("\n/color [HEX]   -   Sets your color in MPP.");

            console_log("\n\nExamples:\n\n/nick foo bar\n/join The Roleplay Room\n/color #ffd700\n");
            return 0;
        }

        if(arg == "--version" || arg == "-v" || arg == "version") {
            console_log("idk.");
            return 0;
        }

        if(arg[0] == '-') break;

        if(i == 1) {
            desiredChannel = arg;
        }
    }

    Client client = Client(getToken());
    printMessage("Hi! Logging in MPP!");

    client.on("hi", [&client](auto msg) {
        client.setChannel("lobby");
        printMessage("Logged in as " + msg["u"]["name"].template get<string>());
    });

    client.on("ch", [&client](auto msg) {
        clear();
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

    thread inputThread([&]() {
        while (true)
        {
            getline(std::cin, currentInput);

            if (currentInput.empty())
                continue;

            if (currentInput[0] == '/')
            {
                if (currentInput.rfind("/nick ", 0) == 0)
                {
                    client.setName(currentInput.substr(6));
                }
                else if (currentInput.rfind("/join ", 0) == 0)
                {
                    client.setChannel(currentInput.substr(6));
                }
                else if (currentInput.rfind("/color ", 0) == 0)
                {
                    client.setColor(currentInput.substr(7));
                }
                else
                {
                    printMessage("Unknown command.");
                }
            }
            else
            {
                client.say(currentInput);
            }

            currentInput.clear();
        }
    });

    inputThread.join();
}