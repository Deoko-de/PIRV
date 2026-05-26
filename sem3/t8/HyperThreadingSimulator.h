#ifndef HYPERTHREADINGSIMULATOR_H
#define HYPERTHREADINGSIMULATOR_H
#include "VirtualThread.h"
#include <random>

class HyperThreadingSimulator {
private:
    VirtualThread logical_processor_a;
    VirtualThread logical_processor_b;

public:
    HyperThreadingSimulator() {
        std::random_device random_source;
        std::mt19937 random_engine(random_source());

        std::uniform_int_distribution<int> value_distribution(1, 50);
        std::uniform_int_distribution<int> duration_distribution(200, 1000);
        std::uniform_int_distribution<int> priority_distribution(1, 10);
        std::uniform_int_distribution<int> steps_distribution(2, 5);

        for (int task_counter = 0; task_counter < 6; ++task_counter) {
            logical_processor_a.addTask(Task(value_distribution(random_engine), 
                                              priority_distribution(random_engine), 
                                              duration_distribution(random_engine), 
                                              steps_distribution(random_engine)));
            logical_processor_b.addTask(Task(value_distribution(random_engine), 
                                              priority_distribution(random_engine), 
                                              duration_distribution(random_engine), 
                                              steps_distribution(random_engine)));
        }
    }

    void start_execution() {
        while (logical_processor_a.hasTasks() || logical_processor_b.hasTasks()) {
            if (logical_processor_a.hasTasks()) logical_processor_a.runStep(1);
            if (logical_processor_b.hasTasks()) logical_processor_b.runStep(2);
        }
    }
};

#endif
