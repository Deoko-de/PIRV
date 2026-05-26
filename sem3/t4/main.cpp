#include <iostream>
#include <random>
#include "TaskTimer.h"

int main() {
    std::random_device random_source;
    std::mt19937 random_generator(random_source());
    std::uniform_int_distribution<int> value_range(1, 1e6);
    std::vector<int> data_array;

    for (int iteration = 0; iteration < 1e5; ++iteration) {
        data_array.push_back(value_range(random_generator));
    }

    TaskTimer performance_meter;

    std::chrono::milliseconds sorting_duration = performance_meter.measureSorting(data_array);
    std::cout << "Sorting execution time: " << sorting_duration.count() << " ms" << std::endl;

    for (int& element : data_array) {
        element = value_range(random_generator);
    }
    
    sorting_duration = performance_meter.measureSorting(data_array);
    std::cout << "Sorting execution time: " << sorting_duration.count() << " ms" << std::endl;

    return 0;
}
