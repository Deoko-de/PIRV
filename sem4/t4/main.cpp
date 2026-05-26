#include <iostream>
#include <thread>
#include <chrono>

#include "Logger.cpp"

int main() {

    Logger<std::string> event_logger("log.txt");

    int worker_count = 5;
    int entries_per_worker = 10;

    for (int worker_id = 0; worker_id < worker_count; ++worker_id) {

        std::thread([worker_id, &event_logger, entries_per_worker]() {

            for (int entry_index = 0; entry_index < entries_per_worker; ++entry_index) {

                event_logger.record(
                    "Message " + std::to_string(entry_index) +
                    " from worker " + std::to_string(worker_id)
                );

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)
                );
            }

        }).detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
