#include "Client/Client.cpp"

int main() {
    Client client = Client("[REDACTED]");

    client.on("hi", [](auto msg) {
        std::cout << "hi";
    });

    client.on("connected", [](auto msg) {
        std::cout << "connected";
    });


    std::cout << "starting" << std::endl;
    client.start();


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}