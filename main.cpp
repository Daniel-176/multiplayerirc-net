#include "Client/Client.cpp"

int main() {
    Client client = Client("[REDACTED]");
    console_log("Hi!");

    client.on("hi", [&client](auto msg) {
        client.setChannel("hello hello");
    });

    client.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}