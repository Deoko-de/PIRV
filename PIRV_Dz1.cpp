#include <iostream>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <boost/thread.hpp>

std::mutex g_lock;
std::condition_variable g_signal;
std::atomic<int> g_completed_segments(0);

long long g_total = 0;
int g_finished_counter = 0;

void compute_row_range(const std::vector<std::vector<int>>& data, int from_row, int to_row) {
    long long partial = 0;
    for (int r = from_row; r < to_row; ++r) {
        for (int c = 0; c < data[r].size(); ++c) {
            partial += data[r][c];
        }
    }

    {
        std::lock_guard<std::mutex> guard(g_lock);
        g_total += partial;
        g_finished_counter++;
        g_completed_segments.fetch_add(1);
    }

    g_signal.notify_one();
}

void compute_block_sum(const std::vector<std::vector<int>>& data, int row_start, int col_start, int block_size) {
    long long block_sum = 0;
    for (int r = row_start; r < row_start + block_size; ++r) {
        for (int c = col_start; c < col_start + block_size; ++c) {
            block_sum += data[r][c];
        }
    }

    {
        std::lock_guard<std::mutex> guard(g_lock);
        g_total += block_sum;
        g_finished_counter++;
        g_completed_segments.fetch_add(1);
    }

    g_signal.notify_one();
}

int main() {
    int dim = 1000;
    int worker_count = 4;
    
    std::random_device rnd_device;
    std::mt19937 rng(rnd_device());
    std::uniform_int_distribution<> value_range(1, 10);

    std::vector<std::vector<int>> grid(dim, std::vector<int>(dim));
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            grid[i][j] = value_range(rng);
        }
    }

    auto timer_start = std::chrono::high_resolution_clock::now();

    std::vector<boost::thread> workers;
    int chunk_rows = dim / worker_count;

    for (int t = 0; t < worker_count; ++t) {
        int start_idx = t * chunk_rows;
        int end_idx = (t == worker_count - 1) ? dim : start_idx + chunk_rows;
        workers.emplace_back(compute_row_range, std::cref(grid), start_idx, end_idx);
    }

    {
        std::unique_lock<std::mutex> locker(g_lock);
        g_signal.wait(locker, [worker_count]{ return g_finished_counter == worker_count; });
    }

    for (auto& thr : workers) {
        thr.join();
    }

    auto timer_end = std::chrono::high_resolution_clock::now();

    std::cout << "Result sum: " << g_total << std::endl;
    std::cout << "Execution time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count() 
              << " ms" << std::endl;
    std::cout << "Processed chunks: " << g_completed_segments.load() << std::endl;
    
    return 0;
}
