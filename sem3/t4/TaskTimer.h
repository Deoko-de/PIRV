#ifndef TASKTIMER_H
#define TASKTIMER_H
#include <chrono>
#include <vector>
#include <algorithm>

class TASKTIMER {
private:
    std::chrono::high_resolution_clock::time_point begin_moment;
    std::chrono::high_resolution_clock::time_point end_moment;
    
public:
    void begin_measurement() {
        begin_moment = std::chrono::high_resolution_clock::now();
    }

    void end_measurement() {
        end_moment = std::chrono::high_resolution_clock::now();
    }

    std::chrono::milliseconds retrieve_elapsed() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_moment - begin_moment);
    }

    std::chrono::milliseconds evaluate_sorting_performance(std::vector<int>& data_container) {
        begin_measurement();
        std::sort(data_container.begin(), data_container.end(), [](int first, int second) {
            return first < second;
        });
        end_measurement();

        return retrieve_elapsed();
    }
};

#endif
