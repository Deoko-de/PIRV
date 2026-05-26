#pragma once
#include <vector>
#include <atomic>
#include "FileChunk.h"

class FileDownload
{
public:

    int file_identifier;
    std::vector<FileChunk> segments;
    std::atomic<int> completed_segments{0};

    FileDownload(int id)
    {
        file_identifier = id;
    }

    bool is_fully_downloaded()
    {
        return completed_segments == segments.size();
    }

    void mark_segment_completed()
    {
        completed_segments++;
    }

    FileDownload(const FileDownload&) = delete;
    FileDownload& operator=(const FileDownload&) = delete;
};
