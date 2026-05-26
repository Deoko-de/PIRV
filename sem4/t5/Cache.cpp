#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>

template<typename KeyType, typename ValueType>
class Cache {
private:
    std::map<KeyType, ValueType> storage;
    mutable std::mutex operation_lock;
    std::condition_variable data_ready;

public:
    inline void insert(const KeyType& identifier, const ValueType& content) {
        {
            std::lock_guard<std::mutex> guard(operation_lock);
            storage[identifier] = content;
            std::cout << "[Thread " << std::this_thread::get_id() 
                      << "] Stored key: " << identifier 
                      << ", value: " << content << std::endl;
        }
        data_ready.notify_all();
    }

    inline ValueType retrieve(const KeyType& identifier) {
        std::unique_lock<std::mutex> locker(operation_lock);
        data_ready.wait(locker, [&]{ return storage.find(identifier) != storage.end(); });

        ValueType fetched_value = storage[identifier];
        std::cout << "[Thread " << std::this_thread::get_id() 
                  << "] Retrieved key: " << identifier 
                  << ", value: " << fetched_value << std::endl;
        return fetched_value;
    }

    void displayAll() const {
        std::lock_guard<std::mutex> guard(operation_lock);
        std::cout << "Cache contents:" << std::endl;
        for (const auto& [entry_key, entry_value] : storage) {
            std::cout << "Key: " << entry_key << ", Value: " << entry_value << std::endl;
        }
    }
};
