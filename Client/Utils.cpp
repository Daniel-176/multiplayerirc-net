#include <iostream>
#include <string>
#include <optional>
#include <map>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>

using namespace std;

long long DateNow() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

void console_log(string message) {
    cout << message << endl;
}