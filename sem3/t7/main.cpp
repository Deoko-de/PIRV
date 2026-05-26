#include "HyperThreadingSimulator.h"

int main() {
    std::vector<std::string> task_list = {"A", "B", "C", "D"};
    HyperThreadingSimulator hyper_thread_simulator(task_list);

    hyper_thread_simulator.start_execution();

    return 0;
}
