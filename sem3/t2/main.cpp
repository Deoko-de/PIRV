#include <iostream>
#include <thread>
#include <chrono>

int main() {
    int countdown_value;
    std::cout << "Enter a positive integer: ";
    std::cin >> countdown_value;
    
    while (countdown_value <= 0) {
        std::cout << "Invalid input. Please try again: ";
        std::cin >> countdown_value;
    }

    for (int remaining = countdown_value; remaining > 0; --remaining) {
        std::cout << "Time left: " << remaining << " sec" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Time's up!" << std::endl;

    return 0;
}
