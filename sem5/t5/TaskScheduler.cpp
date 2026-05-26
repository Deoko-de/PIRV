#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <semaphore>
#include <chrono>
#include <random>

using namespace std;

struct WorkItem
{
    int identifier;
    int slots_needed;
    int processing_time_ms;
    int priority_level;

    chrono::steady_clock::time_point submission_moment;

    bool operator<(const WorkItem& other) const
    {
        return priority_level < other.priority_level;
    }

    void process()
    {
        this_thread::sleep_for(chrono::milliseconds(processing_time_ms));
    }
};

class TaskScheduler
{
private:

    priority_queue<WorkItem> pending_items;

    counting_semaphore<100> capacity_semaphore;

    mutex queue_protector;

    atomic<int> finished_count{0};

    atomic<long long> aggregated_wait_duration{0};

    int max_capacity;

public:

    TaskScheduler(int resource_capacity)
        : capacity_semaphore(resource_capacity), max_capacity(resource_capacity)
    {}

    void add_task(WorkItem item)
    {
        item.submission_moment = chrono::steady_clock::now();

        lock_guard<mutex> guard(queue_protector);

        pending_items.push(item);

        cout << "Work item " << item.identifier
             << " submitted (priority "
             << item.priority_level
             << ", requires "
             << item.slots_needed
             << " resources)\n";
    }

    inline void run_work_item(WorkItem& item)
    {
        item.process();
    }

    void process_worker()
    {
        while(true)
        {
            WorkItem current_item;
            bool item_available = false;

            {
                lock_guard<mutex> guard(queue_protector);

                if(!pending_items.empty())
                {
                    current_item = pending_items.top();
                    pending_items.pop();
                    item_available = true;
                }
            }

            if(!item_available)
            {
                this_thread::yield();
                continue;
            }

            auto start_moment = chrono::steady_clock::now();

            long long wait_duration =
                chrono::duration_cast<chrono::milliseconds>(
                    start_moment - current_item.submission_moment
                ).count();

            aggregated_wait_duration += wait_duration;

            for(int slot = 0; slot < current_item.slots_needed; ++slot)
                capacity_semaphore.acquire();

            cout << "Worker "
                 << this_thread::get_id()
                 << " started item "
                 << current_item.identifier
                 << " (acquired "
                 << current_item.slots_needed
                 << " resources)\n";

            run_work_item(current_item);

            cout << "Worker "
                 << this_thread::get_id()
                 << " completed item "
                 << current_item.identifier
                 << endl;

            for(int release_slot = 0; release_slot < current_item.slots_needed; ++release_slot)
                capacity_semaphore.release();

            finished_count++;

            this_thread::yield();
        }
    }

    double get_average_wait_time()
    {
        int completed = finished_count.load();

        if(completed == 0)
            return 0;

        return (double)aggregated_wait_duration / completed;
    }

    int get_completed_count()
    {
        return finished_count.load();
    }
};
