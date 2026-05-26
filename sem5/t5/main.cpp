#include <iostream>
#include <thread>
#include <random>
#include <chrono>

#include "TaskScheduler.cpp"

int main()
{
    const int MAX_RESOURCES = 4;
    const int WORKER_THREADS = 3;
    const int TOTAL_WORK_ITEMS = 10;

    TaskScheduler task_system(MAX_RESOURCES);

    for(int worker_idx = 0; worker_idx < WORKER_THREADS; ++worker_idx)
    {
        thread([&task_system]{
            task_system.process_worker();
        }).detach();
    }

    mt19937 random_engine(42);

    uniform_int_distribution<int> slot_requirement(1,2);
    uniform_int_distribution<int> processing_duration(200,800);
    uniform_int_distribution<int> priority_assignment(1,5);

    for(int item_counter = 0; item_counter < TOTAL_WORK_ITEMS; ++item_counter)
    {
        WorkItem current_item;

        current_item.identifier = item_counter;
        current_item.slots_needed = slot_requirement(random_engine);
        current_item.processing_time_ms = processing_duration(random_engine);
        current_item.priority_level = priority_assignment(random_engine);

        task_system.add_task(current_item);

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    this_thread::sleep_for(chrono::seconds(8));

    cout << "\nCompleted work items: "
         << task_system.get_completed_count()
         << endl;

    cout << "Average waiting time: "
         << task_system.get_average_wait_time()
         << " ms\n";
}
