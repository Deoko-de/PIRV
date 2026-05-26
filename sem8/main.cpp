#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <random>
#include <algorithm>
#include <execution>
#include <numeric>

struct SensorReading { double measurement; };

class DataRouter {
private:
    std::queue<std::vector<SensorReading>> data_queue;
    std::mutex queue_lock;
    std::condition_variable data_available;
    bool processing_complete = false;

public:
    void submit(std::vector<SensorReading>& batch) {
        std::lock_guard<std::mutex> guard(queue_lock);
        data_queue.push(std::move(batch));
        data_available.notify_one();
    }

    bool retrieve(std::vector<SensorReading>& batch) {
        std::unique_lock locker(queue_lock);
        data_available.wait(locker, [this] { return !data_queue.empty() || processing_complete; });
        if (data_queue.empty()) return false;
        batch = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    void signal_completion() {
        std::lock_guard<std::mutex> guard(queue_lock);
        processing_complete = true;
        data_available.notify_all();
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    DataRouter router;
    std::atomic<double> overall_maximum{0.0};

    std::random_device random_source;
    std::mt19937 random_engine(random_source());
    std::uniform_real_distribution<double> value_distribution(10.0, 389.1);
    std::vector<SensorReading> initial_readings;

    for (int idx = 0; idx < 1000; ++idx) {
        initial_readings.push_back({value_distribution(random_engine)});
    }

    std::jthread producer_thread([&] {
        router.submit(initial_readings);
        router.signal_completion();
    });

    std::vector<std::jthread> worker_pool;
    for (int worker_id = 0; worker_id < 4; ++worker_id) {
        worker_pool.emplace_back([&] {
            std::vector<SensorReading> current_batch;
            while(router.retrieve(current_batch)) {
                std::sort(std::execution::par, current_batch.begin(), current_batch.end(),
                    [](auto& first, auto& second) { return first.measurement < second.measurement; });
                
                double total_sum = std::transform_reduce(std::execution::par_unseq,
                    current_batch.begin(), current_batch.end(), 0.0, std::plus<>(),
                    [](auto& entry) { return entry.measurement; });

                std::vector<double> prefix_sums(current_batch.size());
                std::transform_inclusive_scan(std::execution::par, current_batch.begin(), current_batch.end(),
                    prefix_sums.begin(), std::plus<double>(), [](auto& entry) { return entry.measurement; });

                double batch_maximum = current_batch.back().measurement;
                double current_max = overall_maximum.load();
                while (batch_maximum > current_max && !overall_maximum.compare_exchange_weak(current_max, batch_maximum));

                std::cout << "Batch processed successfully. Total: " << total_sum << ", Peak: " << batch_maximum << std::endl;
            }
        });
    }

    return 0;
}
