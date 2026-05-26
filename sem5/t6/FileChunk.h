#pragma once
#include <chrono>
#include <thread>

struct FileChunk
{
    int segment_index;
    int parent_file_id;
    size_t chunk_size;

    void download()
    {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100 + chunk_size)
        );
    }
};
