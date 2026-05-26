#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

void multiplyByScalarCPU(const std::vector<float>& input, std::vector<float>& output, float scalar) {
    for (size_t position = 0; position < input.size(); ++position) {
        output[position] = input[position] * scalar;
    }
}

int main() {
    const size_t ARRAY_SIZE = 1000000;
    const float SCALAR_VALUE = 2.5f;

    std::vector<float> source_data(ARRAY_SIZE);
    std::vector<float> cpu_result(ARRAY_SIZE, 0.0f);
    std::vector<float> gpu_result(ARRAY_SIZE, 0.0f);

    for (size_t idx = 0; idx < ARRAY_SIZE; ++idx) {
        source_data[idx] = static_cast<float>(idx);
    }

    auto cpu_start = std::chrono::high_resolution_clock::now();
    multiplyByScalarCPU(source_data, cpu_result, SCALAR_VALUE);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_duration = cpu_end - cpu_start;

    try {
        sycl::queue compute_queue(sycl::default_selector_v);
        std::cout << "Computations running on device: " 
                  << compute_queue.get_device().get_info<sycl::info::device::name>() << "\n";

        auto gpu_start = std::chrono::high_resolution_clock::now();

        {
            sycl::buffer<float, 1> buffer_input(source_data.data(), sycl::range<1>(ARRAY_SIZE));
            sycl::buffer<float, 1> buffer_output(gpu_result.data(), sycl::range<1>(ARRAY_SIZE));

            compute_queue.submit([&](sycl::handler& handler) {
                auto input_accessor = buffer_input.get_access<sycl::access::mode::read>(handler);
                auto output_accessor = buffer_output.get_access<sycl::access::mode::write>(handler);

                handler.parallel_for(sycl::range<1>(ARRAY_SIZE), [=](sycl::id<1> element_id) {
                    output_accessor[element_id] = input_accessor[element_id] * SCALAR_VALUE;
                });
            });

            compute_queue.wait();
        }

        auto gpu_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gpu_duration = gpu_end - gpu_start;

        bool is_identical = true;
        for (size_t position = 0; position < ARRAY_SIZE; ++position) {
            if (std::abs(cpu_result[position] - gpu_result[position]) > 1e-4f) {
                is_identical = false;
                break;
            }
        }

        std::cout << "Verification result: " << (is_identical ? "PASSED" : "FAILED") << "\n";
        std::cout << "CPU processing time: " << cpu_duration.count() << " seconds\n";
        std::cout << "SYCL processing time: " << gpu_duration.count() << " seconds\n";
        std::cout << "Performance gain: " << cpu_duration.count() / gpu_duration.count() << "x\n";

    }
    catch (const sycl::exception& error) {
        std::cerr << "SYCL error occurred: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
