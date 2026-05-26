#include <iostream>
#include <thread>
#include <chrono>

#include "Buffer.cpp"

int main() {

    Buffer<int> shared_queue(5);

    int items_per_producer = 20;
    int items_per_consumer = 20;

    int producer_threads = 3;
    int consumer_threads = 3;

    for (int producer_id = 0; producer_id < producer_threads; ++producer_id) {

        std::thread([&shared_queue, items_per_producer, producer_id]() {

            for (int item_index = 0; item_index < items_per_producer; ++item_index) {

                shared_queue.insert_item(item_index + producer_id * 100);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

        }).detach();
    }

    for (int consumer_id = 0; consumer_id < consumer_threads; ++consumer_id) {

        std::thread([&shared_queue, items_per_consumer]() {

            for (int item_index = 0; item_index < items_per_consumer; ++item_index) {

                shared_queue.retrieve_item();

                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }

        }).detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
