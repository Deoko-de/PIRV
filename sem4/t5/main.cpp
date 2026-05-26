#include "Cache.cpp"
#include <thread>
#include <chrono>
#include <string>

int main() {
    Cache<int, std::string> data_cache;
    
    auto producer = [&data_cache]() {
        for (int key_id = 1; key_id <= 5; ++key_id) {
            data_cache.insert(key_id, std::to_string(key_id));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::this_thread::yield();
        }
    };

    auto consumer = [&data_cache]() {
        for (int key_id = 1; key_id <= 5; ++key_id) {
            std::string retrieved_value = data_cache.retrieve(key_id);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::this_thread::yield();
        }
    };

    std::thread producer_thread(producer);
    std::thread consumer_one(consumer);
    std::thread consumer_two(consumer);

    producer_thread.detach();
    consumer_one.detach();
    consumer_two.detach();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    data_cache.displayAll();

    return 0;
}
