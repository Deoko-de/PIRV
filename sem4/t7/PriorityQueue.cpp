#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

template<typename DataType>
struct RankedItem {
    DataType content;
    int rank;

    bool operator<(const RankedItem& other) const {
        return rank < other.rank;
    }
};

template<typename ElementType>
class PriorityQueue {
private:
    std::priority_queue<RankedItem<ElementType>> internal_queue;
    mutable std::mutex operation_lock;
    std::condition_variable state_change;

public:
    void insert(ElementType item_value, int item_priority) {
        {
            std::lock_guard<std::mutex> guard(operation_lock);
            internal_queue.push({item_value, item_priority});
            std::cout << "[Thread " << std::this_thread::get_id()
                      << "] Added element: " << item_value
                      << " with priority " << item_priority << std::endl;
        }
        state_change.notify_one();
    }

    ElementType extract() {
        std::unique_lock<std::mutex> locker(operation_lock);
        state_change.wait(locker, [this]() { return !internal_queue.empty(); });

        auto highest_item = internal_queue.top();
        internal_queue.pop();
        std::cout << "[Thread " << std::this_thread::get_id()
                  << "] Extracted element: " << highest_item.content
                  << " with priority " << highest_item.rank << std::endl;
        return highest_item.content;
    }

    size_t current_size() const {
        std::lock_guard<std::mutex> guard(operation_lock);
        return internal_queue.size();
    }
};
