#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <random>

using namespace std;

template<typename DataType>
class SemaphoreBuffer
{
private:

    vector<vector<DataType>> storage_buffers;

    vector<counting_semaphore<>> empty_slots;
    vector<counting_semaphore<>> occupied_slots;

    vector<mutex> buffer_locks;

    int max_buffer_capacity;

public:

    SemaphoreBuffer(int buffer_count, int capacity_per_buffer)
        : max_buffer_capacity(capacity_per_buffer)
    {
        storage_buffers.resize(buffer_count);

        for(int idx = 0; idx < buffer_count; ++idx)
        {
            empty_slots.emplace_back(capacity_per_buffer);
            occupied_slots.emplace_back(0);
        }

        buffer_locks.resize(buffer_count);
    }
    
    void insert_item(DataType item, int target_buffer, int wait_timeout_ms)
    {
        auto thread_id = this_thread::get_id();

        bool acquired = empty_slots[target_buffer].try_acquire_for(
            chrono::milliseconds(wait_timeout_ms)
        );

        if(!acquired)
        {
            lock_guard<mutex> guard(buffer_locks[target_buffer]);

            cout << "Thread " << thread_id
                 << " buffer " << target_buffer
                 << ": write timeout expired\n";

            return;
        }

        {
            lock_guard<mutex> guard(buffer_locks[target_buffer]);

            storage_buffers[target_buffer].push_back(item);

            cout << "Thread " << thread_id
                 << " produced " << item
                 << " -> buffer " << target_buffer
                 << endl;
        }

        occupied_slots[target_buffer].release();
    }
    
    DataType extract_item(int source_buffer, int wait_timeout_ms)
    {
        auto thread_id = this_thread::get_id();

        bool acquired = occupied_slots[source_buffer].try_acquire_for(
            chrono::milliseconds(wait_timeout_ms)
        );

        if(!acquired)
        {
            lock_guard<mutex> guard(buffer_locks[source_buffer]);

            cout << "Thread " << thread_id
                 << " buffer " << source_buffer
                 << ": read timeout expired\n";

            return DataType();
        }

        DataType retrieved_value;

        {
            lock_guard<mutex> guard(buffer_locks[source_buffer]);

            retrieved_value = storage_buffers[source_buffer].back();
            storage_buffers[source_buffer].pop_back();

            cout << "Thread " << thread_id
                 << " consumed " << retrieved_value
                 << " <- buffer " << source_buffer
                 << endl;
        }

        empty_slots[source_buffer].release();

        return retrieved_value;
    }
};

void generate_producer(SemaphoreBuffer<int>& shared_buffer)
{
    mt19937 random_gen(42);
    uniform_int_distribution<> buffer_selector(0,2);
    uniform_int_distribution<> value_generator(1,100);

    int selected_buffer = buffer_selector(random_gen);
    int produced_value = value_generator(random_gen);

    shared_buffer.insert_item(produced_value, selected_buffer, 500);

    this_thread::yield();
}

void generate_consumer(SemaphoreBuffer<int>& shared_buffer)
{
    mt19937 random_gen(84);
    uniform_int_distribution<> buffer_selector(0,2);

    int selected_buffer = buffer_selector(random_gen);

    shared_buffer.extract_item(selected_buffer, 500);

    this_thread::yield();
}
