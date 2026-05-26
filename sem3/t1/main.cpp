#include <iostream>
#include <chrono>

long long compute_sum_up_to(int limit) {
    long long accumulated = 0;

    for (int counter = 1; counter <= limit; ++counter) {
        accumulated += counter;
    }

    return accumulated;
}

int main() {
    int target_number;
    std::cout << "Enter a positive integer: ";
    std::cin >> target_number;
    
    while (target_number <= 0) {
        std::cout << "Invalid input, please try again: ";
        std::cin >> target_number;
    }

    auto timer_start = std::chrono::high_resolution_clock::now();

    long long final_sum = compute_sum_up_to(target_number);

    auto timer_end = std::chrono::high_resolution_clock::now();

    std::cout << "Result: " << final_sum << std::endl;
    std::cout << "Execution time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count() 
              << " ms" << std::endl;
    
    return 0;
}
