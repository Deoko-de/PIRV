#include <iostream>
#include <thread>
#include <chrono>

#include "SemaphoreBuffer.cpp"

int main()
{
    SemaphoreBuffer<int> shared_buffer(3, 5);

    for(int producer_index = 0; producer_index < 10; ++producer_index)
    {
        thread([&shared_buffer]{
            generate_producer(shared_buffer);
        }).detach();
    }

    for(int consumer_index = 0; consumer_index < 10; ++consumer_index)
    {
        thread([&shared_buffer]{
            generate_consumer(shared_buffer);
        }).detach();
    }

    this_thread::sleep_for(chrono::seconds(5));
}
