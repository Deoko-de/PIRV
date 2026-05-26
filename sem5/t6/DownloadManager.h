#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <semaphore>
#include "FileChunk.h"
#include "FileDownload.h"

class DownloadManager
{
private:

    std::queue<DataSegment> segment_queue;

    std::counting_semaphore<100> active_tasks;
    std::counting_semaphore<100> chunk_operations;

    std::mutex task_queue_lock;

    std::atomic<int> finished_file_count{0};

    std::vector<FileDownload> stored_files;

public:

    DownloadManager(int max_files, int max_chunks);

    void add_file(int file_identifier, int total_chunks);

    void worker_thread();

    inline void process_segment(DataSegment segment);

    int get_completed_files();
};
