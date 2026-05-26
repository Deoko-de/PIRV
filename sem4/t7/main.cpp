#include "PriorityQueue.cpp"
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <string>

int main() {
    PriorityQueue<std::string> task_queue;

    auto data_producer = [&task_queue](int worker_identifier) {
        for (int item_counter = 1; item_counter <= 5; ++item_counter) {
            int assigned_priority = rand() % 100;
            task_queue.insert("Item_" + std::to_string(item_counter) + "_thread_" + std::to_string(worker_identifier), assigned_priority);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::this_thread::yield();
        }
    };

    auto data_consumer = [&task_queue]() {
        for (int item_counter = 0; item_counter < 5; ++item_counter) {
            std::string extracted_value = task_queue.extract();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::this_thread::yield();
        }
    };

    std::thread producer_one(data_producer, 1);
    std::thread producer_two(data_producer, 2);
    std::thread consumer_one(data_consumer);
    std::thread consumer_two(data_consumer);

    producer_one.detach();
    producer_two.detach();
    consumer_one.detach();
    consumer_two.detach();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Queue size after threads execution: " << task_queue.current_size() << std::endl;

    return 0;
}
