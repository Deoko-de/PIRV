#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <semaphore>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>

using namespace std;

struct QueuedThread {
    int priority_level;
    thread::id thread_identifier;

    bool operator<(const QueuedThread& other) const {
        return priority_level < other.priority_level;
    }
};

template<typename ResourceType>
class ObjectPool
{
private:

    vector<ResourceType> available_objects;

    counting_semaphore<> pool_semaphore;
    mutex access_lock;
    condition_variable availability_signal;

    atomic<int> timeout_counter{0};

    priority_queue<QueuedThread> pending_queue;

public:

    ObjectPool(vector<ResourceType> initial_objects)
        : available_objects(initial_objects),
          pool_semaphore(initial_objects.size())
    {}

    ResourceType request_resource(int priority_value, int wait_duration_ms)
    {
        thread::id current_tid = this_thread::get_id();

        if(!pool_semaphore.try_acquire_for(chrono::milliseconds(wait_duration_ms)))
        {
            timeout_counter++;

            std::lock_guard<std::mutex> guard(access_lock);
            cout << "Thread " << current_tid
                 << " with priority " << priority_value
                 << " exceeded waiting time\n";

            throw runtime_error("Timeout exceeded\n");
        }

        unique_lock<mutex> locker(access_lock);

        pending_queue.push({priority_value, current_tid});

        availability_signal.wait(locker, [&]{
            return pending_queue.top().thread_identifier == current_tid;
        });

        ResourceType acquired_object = available_objects.back();
        available_objects.pop_back();

        pending_queue.pop();

        cout << "Thread " << current_tid
             << " with priority " << priority_value
             << " acquired resource\n";

        return acquired_object;
    }

    void return_resource(ResourceType released_object, int priority_value)
    {
        {
            lock_guard<mutex> guard(access_lock);

            available_objects.push_back(released_object);

            cout << "Thread "
                 << this_thread::get_id()
                 << " with priority " << priority_value
                 << " released resource\n";
        }

        pool_semaphore.release();

        availability_signal.notify_all();
    }

    int get_timeout_count()
    {
        return timeout_counter.load();
    }
};

void perform_work(ObjectPool<int>& object_pool, int priority_level)
{
    try
    {
        int acquired_resource = object_pool.request_resource(priority_level, 500);

        this_thread::sleep_for(chrono::milliseconds(300));

        object_pool.return_resource(acquired_resource, priority_level);
    }
    catch(const std::exception& error)
    {
        std::cout << error.what();
    }

    this_thread::yield();
}

int main()
{
    vector<int> initial_pool = {1,2,3};

    ObjectPool<int> resource_pool(initial_pool);

    std::random_device random_source;
    std::mt19937 random_engine(random_source());
    std::uniform_int_distribution<int> priority_distribution(0,4);

    for(int worker_index = 0; worker_index < 10; ++worker_index)
    {
        thread([&resource_pool, worker_index, &random_engine, &priority_distribution]{
            perform_work(resource_pool, priority_distribution(random_engine));
        }).detach();
    }

    this_thread::sleep_for(chrono::seconds(5));

    cout << "Failed attempts count: "
         << resource_pool.get_timeout_count() << endl;
}
