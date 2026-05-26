#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename NumericType>
class ParallelAggregator {

private:
    std::vector<NumericType> elements;
    size_t worker_count;

    NumericType accumulated_result = 0;

    std::mutex result_lock;
    std::condition_variable completion_signal;

    int completed_workers = 0;

    inline NumericType compute_chunk_sum(size_t start_position, size_t end_position) {

        NumericType partial_aggregate = 0;

        for (size_t index = start_position; index < end_position; ++index) {

            partial_aggregate += elements[index];

            if (index % 50 == 0) {
                std::this_thread::yield();
            }
        }

        return partial_aggregate;
    }

public:

    ParallelAggregator(std::vector<NumericType> input_data, size_t thread_pool_size)
        : elements(input_data), worker_count(thread_pool_size) {}

    NumericType calculate_total() {

        size_t chunk_dimension = elements.size() / worker_count;

        for (size_t worker_index = 0; worker_index < worker_count; ++worker_index) {

            size_t chunk_begin = worker_index * chunk_dimension;
            size_t chunk_end;
            
            if (worker_index == worker_count - 1) {
                chunk_end = elements.size();
            } else {
                chunk_end = chunk_begin + chunk_dimension;
            }

            std::thread([this, chunk_begin, chunk_end]() {

                NumericType partial_sum = compute_chunk_sum(chunk_begin, chunk_end);

                {
                    std::lock_guard<std::mutex> guard(result_lock);

                    accumulated_result += partial_sum;

                    std::cout
                        << "Worker "
                        << std::this_thread::get_id()
                        << " segment sum = "
                        << partial_sum
                        << std::endl;

                    ++completed_workers;
                }

                completion_signal.notify_one();

            }).detach();
        }

        std::unique_lock<std::mutex> locker(result_lock);

        completion_signal.wait(locker, [this]() {
            return completed_workers == worker_count;
        });

        return accumulated_result;
    }
};

int main() {

    size_t array_dimension;
    size_t thread_pool_dimension;

    std::cout << "Enter number of array elements: ";
    std::cin >> array_dimension;

    std::cout << "Enter number of threads: ";
    std::cin >> thread_pool_dimension;

    std::vector<int> number_sequence;

    for (size_t position = 1; position <= array_dimension; ++position) {
        number_sequence.push_back(static_cast<int>(position));
    }

    ParallelAggregator<int> aggregator(number_sequence, thread_pool_dimension);

    int final_result = aggregator.calculate_total();

    std::cout << "Total sum = " << final_result << std::endl;

    return 0;
}
