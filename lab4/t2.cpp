#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <chrono>

void applyThresholdCPU(const std::vector<unsigned char>& input_data, std::vector<unsigned char>& output_data, int image_width, int image_height, unsigned char threshold_value) {
    for (int row = 0; row < image_height; ++row) {
        for (int col = 0; col < image_width; ++col) {
            int linear_index = row * image_width + col;
            output_data[linear_index] = (input_data[linear_index] > threshold_value) ? 255 : 0;
        }
    }
}

int main() {
    const int IMG_WIDTH = 1024;
    const int IMG_HEIGHT = 1024;
    const size_t TOTAL_PIXELS = IMG_WIDTH * IMG_HEIGHT;
    const unsigned char THRESHOLD = 128;

    std::vector<unsigned char> source_pixels(TOTAL_PIXELS);
    std::vector<unsigned char> cpu_binary(TOTAL_PIXELS, 0);
    std::vector<unsigned char> gpu_binary(TOTAL_PIXELS, 0);

    for (size_t idx = 0; idx < TOTAL_PIXELS; ++idx) {
        source_pixels[idx] = static_cast<unsigned char>(idx % 256);
    }

    auto cpu_start = std::chrono::high_resolution_clock::now();
    applyThresholdCPU(source_pixels, cpu_binary, IMG_WIDTH, IMG_HEIGHT, THRESHOLD);
    auto cpu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpu_elapsed = cpu_end - cpu_start;

    try {
        sycl::queue compute_queue(sycl::default_selector_v);
        std::cout << "Binarization running on: " 
                  << compute_queue.get_device().get_info<sycl::info::device::name>() << "\n";

        auto gpu_start = std::chrono::high_resolution_clock::now();

        {
            sycl::buffer<unsigned char, 2> input_buffer(source_pixels.data(), sycl::range<2>(IMG_HEIGHT, IMG_WIDTH));
            sycl::buffer<unsigned char, 2> output_buffer(gpu_binary.data(), sycl::range<2>(IMG_HEIGHT, IMG_WIDTH));

            compute_queue.submit([&](sycl::handler& handler) {
                auto input_accessor = input_buffer.get_access<sycl::access::mode::read>(handler);
                auto output_accessor = output_buffer.get_access<sycl::access::mode::write>(handler);

                sycl::range<2> global_dimensions(IMG_HEIGHT, IMG_WIDTH);
                sycl::range<2> local_dimensions(16, 16);
                sycl::nd_range<2> parallel_range(global_dimensions, local_dimensions);

                handler.parallel_for(parallel_range, [=](sycl::nd_item<2> work_item) {
                    sycl::id<2> global_position = work_item.get_global_id();

                    if (input_accessor[global_position] > THRESHOLD) {
                        output_accessor[global_position] = 255;
                    }
                    else {
                        output_accessor[global_position] = 0;
                    }
                });
            });

            compute_queue.wait();
        }

        auto gpu_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gpu_elapsed = gpu_end - gpu_start;

        bool results_match = true;
        for (size_t position = 0; position < TOTAL_PIXELS; ++position) {
            if (cpu_binary[position] != gpu_binary[position]) {
                results_match = false;
                break;
            }
        }

        std::cout << "Validation status: " << (results_match ? "PASSED" : "FAILED") << "\n";
        std::cout << "CPU execution time: " << cpu_elapsed.count() << " seconds\n";
        std::cout << "SYCL 2D ND-range time: " << gpu_elapsed.count() << " seconds\n";
        std::cout << "Speedup ratio: " << cpu_elapsed.count() / gpu_elapsed.count() << "x\n";

    }
    catch (const sycl::exception& error) {
        std::cerr << "SYCL error detected: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
