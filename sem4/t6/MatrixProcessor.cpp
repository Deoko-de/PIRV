#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

template<typename DataType>
class MatrixProcessor {
private:
    std::vector<std::vector<DataType>> grid;
    size_t worker_count;
    mutable std::mutex operation_lock;
    std::condition_variable completion_signal;
    size_t completed_workers = 0;

public:
    MatrixProcessor(const std::vector<std::vector<DataType>>& source_matrix, size_t thread_pool_size)
        : grid(source_matrix), worker_count(thread_pool_size) {}

    void execute_transformation(std::function<DataType(DataType)> transformation_function) {

        size_t total_rows = grid.size();
        if (total_rows == 0) return;

        size_t active_workers = std::min(worker_count, total_rows);
        size_t rows_per_worker = (total_rows + active_workers - 1) / active_workers;

        for (size_t worker_idx = 0; worker_idx < active_workers; ++worker_idx) {

            size_t segment_start = worker_idx * rows_per_worker;
            size_t segment_end = std::min(segment_start + rows_per_worker, total_rows);

            std::thread([this, segment_start, segment_end, &transformation_function]() {

                {
                    std::lock_guard<std::mutex> guard(operation_lock);
                    std::cout << "[Thread " << std::this_thread::get_id()
                              << "] Started processing rows from "
                              << segment_start << " to " << segment_end - 1 << std::endl;
                }

                for (size_t row_pos = segment_start; row_pos < segment_end; ++row_pos) {
                    for (size_t col_pos = 0; col_pos < grid[row_pos].size(); ++col_pos) {
                        grid[row_pos][col_pos] = transformation_function(grid[row_pos][col_pos]);
                        std::this_thread::yield();
                    }
                }

                {
                    std::lock_guard<std::mutex> guard(operation_lock);
                    ++completed_workers;
                    std::cout << "[Thread " << std::this_thread::get_id()
                              << "] Finished processing rows from "
                              << segment_start << " to " << segment_end - 1 << std::endl;
                }

                completion_signal.notify_one();

            }).detach();
        }

        std::unique_lock<std::mutex> locker(operation_lock);
        completion_signal.wait(locker, [&]() { return completed_workers == active_workers; });
    }

    void display_matrix() const {
        std::lock_guard<std::mutex> guard(operation_lock);
        std::cout << "Matrix contents: " << std::endl;
        for (const auto& row_data : grid) {
            for (const auto& element : row_data) {
                std::cout << element << " ";
            }
            std::cout << std::endl;
        }
    }
};
