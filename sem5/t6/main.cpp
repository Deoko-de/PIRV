#include <iostream>
#include <thread>
#include <random>
#include "DownloadManager.h"

int main()
{
    const int MAX_CONCURRENT_FILES = 2;
    const int MAX_SIMULTANEOUS_SEGMENTS = 4;
    const int WORKER_THREADS = 4;

    DownloadManager task_manager(MAX_CONCURRENT_FILES, MAX_SIMULTANEOUS_SEGMENTS);

    for(int thread_idx = 0; thread_idx < WORKER_THREADS; ++thread_idx)
    {
        std::thread([&task_manager]{
            task_manager.worker_thread();
        }).detach();
    }

    std::mt19937 random_engine(42);

    std::uniform_int_distribution<int> segment_distribution(3,6);
    std::uniform_int_distribution<int> size_distribution(50,200);

    for(int file_counter = 0; file_counter < 5; ++file_counter)
    {
        task_manager.add_file(file_counter, 5);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(200)
        );
    }

    std::this_thread::sleep_for(
        std::chrono::seconds(8)
    );

    std::cout << "\nCompleted files: "
              << task_manager.get_completed_files()
              << std::endl;
}
