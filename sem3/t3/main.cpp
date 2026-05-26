#include <iostream>
#include <chrono>

int main() {
    int input_seconds;
    std::cout << "Enter a positive integer: ";
    std::cin >> input_seconds;
    
    while (input_seconds <= 0) {
        std::cout << "Invalid input. Please try again: ";
        std::cin >> input_seconds;
    }

    std::chrono::seconds total_seconds(input_seconds);

    std::chrono::hours hours_component = std::chrono::duration_cast<std::chrono::hours>(total_seconds);
    total_seconds -= std::chrono::duration_cast<std::chrono::seconds>(hours_component);
    
    std::chrono::minutes minutes_component = std::chrono::duration_cast<std::chrono::minutes>(total_seconds);
    total_seconds -= std::chrono::duration_cast<std::chrono::seconds>(minutes_component);
    
    std::chrono::seconds seconds_component = total_seconds;

    std::cout << hours_component.count() << " hour(s) " 
              << minutes_component.count() << " minute(s) " 
              << seconds_component.count() << " second(s)" << std::endl;

    return 0;
}
