#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <queue>
#include <random>

using namespace std;

struct PrintTask
{
    string filename;
    int importance;

    bool operator<(const PrintTask& other) const
    {
        return importance < other.importance;
    }
};

class PrinterQueue
{
private:

    int available_printers;

    counting_semaphore<100> printer_semaphore;

    mutex queue_lock;

    priority_queue<PrintTask> pending_tasks;

public:

    PrinterQueue(int printer_count) : available_printers(printer_count), printer_semaphore(printer_count) {}

    void submit_job(string document, int priority_value, int max_wait_ms)
    {
        auto thread_id = this_thread::get_id();

        PrintTask new_job{document, priority_value};

        {
            lock_guard<mutex> guard(queue_lock);
            pending_tasks.push(new_job);
        }

        bool acquired = printer_semaphore.try_acquire_for(
            chrono::milliseconds(max_wait_ms)
        );

        if(!acquired)
        {
            lock_guard<mutex> guard(queue_lock);

            cout << "Thread " << thread_id
                 << " task " << document
                 << " priority " << priority_value
                 << " TIMEOUT (returned to queue)\n";

            pending_tasks.push(new_job);

            return;
        }

        PrintTask current_task;

        {
            lock_guard<mutex> guard(queue_lock);

            if(pending_tasks.empty())
            {
                printer_semaphore.release();
                return;
            }

            current_task = pending_tasks.top();
            pending_tasks.pop();

            cout << "Thread " << thread_id
                 << " printing "
                 << current_task.filename
                 << " priority "
                 << current_task.importance
                 << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(500));

        thread_local mt19937 random_gen(random_device{}());
        uniform_int_distribution<int> interruption_chance(0, 4);
        bool is_interrupted = interruption_chance(random_gen) == 0;

        if(is_interrupted)
        {
            lock_guard<mutex> guard(queue_lock);

            cout << "Thread " << thread_id
                 << " task " << current_task.filename
                 << " INTERRUPTED\n";

            pending_tasks.push(current_task);
        }
        else
        {
            lock_guard<mutex> guard(queue_lock);

            cout << "Thread " << thread_id
                 << " completed "
                 << current_task.filename
                 << endl;
        }

        printer_semaphore.release();

        this_thread::yield();
    }
};

void perform_work(PrinterQueue& print_system, int worker_identifier)
{
    thread_local mt19937 random_generator(42 + worker_identifier);

    uniform_int_distribution<int> priority_range(1,5);

    int task_priority = priority_range(random_generator);

    string document_name = "doc_" + to_string(worker_identifier);

    print_system.submit_job(document_name, task_priority, 500);
}

int main()
{
    PrinterQueue print_queue(2);

    for(int worker_index = 0; worker_index < 10; ++worker_index)
    {
        thread([&print_queue, worker_index]{
            perform_work(print_queue, worker_index);
        }).detach();
    }

    this_thread::sleep_for(chrono::seconds(5));
}
