#include "DownloadManager.h"
#include <iostream>
#include <thread>

DownloadManager::DownloadManager(int max_files, int max_chunks)
    : active_tasks(max_files),
      chunk_operations(max_chunks)
{}

void DownloadManager::add_file(int file_identifier, int total_chunks)
{
    active_tasks.acquire();

    std::lock_guard<std::mutex> guard(task_queue_lock);

    stored_files.emplace_back(file_identifier);

    auto& current_file = stored_files.back();

    for(int chunk_index = 0; chunk_index < total_chunks; ++chunk_index)
    {
        DataSegment segment;

        segment.segment_id = chunk_index;
        segment.file_reference = file_identifier;
        segment.data_size = 100;

        current_file.segments.push_back(segment);
        segment_queue.push(segment);
    }

    std::cout << "File "
              << current_file.file_reference
              << " added with "
              << current_file.segments.size()
              << " segments\n";
}

inline void DownloadManager::process_segment(DataSegment segment)
{
    segment.download();
}

void DownloadManager::worker_thread()
{
    while(true)
    {
        DataSegment current_segment;
        bool segment_available = false;

        {
            std::lock_guard<std::mutex> guard(task_queue_lock);

            if(!segment_queue.empty())
            {
                current_segment = segment_queue.front();
                segment_queue.pop();
                segment_available = true;
            }
        }

        if(!segment_available)
        {
            std::this_thread::yield();
            continue;
        }

        chunk_operations.acquire();

        std::cout << "Worker "
                  << std::this_thread::get_id()
                  << " downloading file "
                  << current_segment.file_reference
                  << " segment "
                  << current_segment.segment_id
                  << std::endl;

        process_segment(current_segment);

        std::cout << "Worker "
                  << std::this_thread::get_id()
                  << " completed file "
                  << current_segment.file_reference
                  << " segment "
                  << current_segment.segment_id
                  << std::endl;

        chunk_operations.release();

        bool file_fully_downloaded = false;

        {
            std::lock_guard<std::mutex> guard(task_queue_lock);

            auto& target_file = stored_files[current_segment.file_reference];

            target_file.mark_segment_completed();

            if(target_file.is_fully_downloaded())
            {
                finished_file_count++;

                file_fully_downloaded = true;

                std::cout << "File "
                          << target_file.file_reference
                          << " completely downloaded\n";
            }
        }

        if(file_fully_downloaded)
            active_tasks.release();

        std::this_thread::yield();
    }
}

int DownloadManager::get_completed_files()
{
    return finished_file_count.load();
}
