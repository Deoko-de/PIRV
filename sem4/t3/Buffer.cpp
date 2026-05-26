#include <vector>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>

template<typename DataType>
class Buffer {

private:
    std::vector<DataType> storage;
    size_t max_capacity;

    std::mutex access_lock;
    std::condition_variable state_change;

public:

    Buffer(size_t queue_capacity) : max_capacity(queue_capacity) {}

    void insert_item(DataType item) {

        std::unique_lock<std::mutex> locker(access_lock);

        while (storage.size() >= max_capacity) {
            std::this_thread::yield();
            state_change.wait(locker);
        }

        storage.push_back(item);

        std::cout << "Produced: "
                  << item
                  << " by thread "
                  << std::this_thread::get_id()
                  << std::endl;

        state_change.notify_one();
    }

    DataType retrieve_item() {

        std::unique_lock<std::mutex> locker(access_lock);

        while (storage.empty()) {
            std::this_thread::yield();
            state_change.wait(locker);
        }

        DataType retrieved = storage.front();
        storage.erase(storage.begin());

        std::cout << "Consumed: "
                  << retrieved
                  << " by thread "
                  << std::this_thread::get_id()
                  << std::endl;

        state_change.notify_one();

        return retrieved;
    }
};
