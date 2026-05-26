#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std::chrono;

std::vector<int> bubble_sort_algorithm(const std::vector<int>& source_data) {
    std::vector<int> working_copy = source_data;
    int data_size = working_copy.size();

    for (int outer_idx = 0; outer_idx < data_size - 1; ++outer_idx) {
        for (int inner_idx = 0; inner_idx < data_size - outer_idx - 1; ++inner_idx) {
            if (working_copy[inner_idx] > working_copy[inner_idx + 1]) {
                std::swap(working_copy[inner_idx], working_copy[inner_idx + 1]);
            }
        }
    }

    return working_copy;
}

std::vector<int> insertion_sort_algorithm(const std::vector<int>& source_data) {
    std::vector<int> working_copy = source_data;
    int data_size = working_copy.size();

    for (int current_pos = 1; current_pos < data_size; ++current_pos) {
        int current_value = working_copy[current_pos];
        int scan_pos = current_pos - 1;

        while (scan_pos >= 0 && working_copy[scan_pos] > current_value) {
            working_copy[scan_pos + 1] = working_copy[scan_pos];
            --scan_pos;
        }

        working_copy[scan_pos + 1] = current_value;
    }

    return working_copy;
}

void merge_segments(std::vector<int>& target, int left_bound, int middle_point, int right_bound) {
    int left_size = middle_point - left_bound + 1;
    int right_size = right_bound - middle_point;

    std::vector<int> left_segment(left_size);
    std::vector<int> right_segment(right_size);

    for (int idx = 0; idx < left_size; ++idx) left_segment[idx] = target[left_bound + idx];
    for (int idx = 0; idx < right_size; ++idx) right_segment[idx] = target[middle_point + idx + 1];

    int left_ptr = 0, right_ptr = 0, target_ptr = left_bound;

    while (left_ptr < left_size && right_ptr < right_size) {
        if (left_segment[left_ptr] <= right_segment[right_ptr]) {
            target[target_ptr++] = left_segment[left_ptr++];
        } else {
            target[target_ptr++] = right_segment[right_ptr++];
        }
    }

    while (left_ptr < left_size) target[target_ptr++] = left_segment[left_ptr++];
    while (right_ptr < right_size) target[target_ptr++] = right_segment[right_ptr++];
}

void merge_sort_recursive(std::vector<int>& target, int left_bound, int right_bound) {
    if (left_bound < right_bound) {
        int middle_point = (left_bound + right_bound) / 2;

        merge_sort_recursive(target, left_bound, middle_point);
        merge_sort_recursive(target, middle_point + 1, right_bound);

        merge_segments(target, left_bound, middle_point, right_bound);
    }
}

std::vector<int> merge_sort_algorithm(const std::vector<int>& source_data) {
    std::vector<int> working_copy = source_data;
    merge_sort_recursive(working_copy, 0, working_copy.size() - 1);
    return working_copy;
}

int split_array(std::vector<int>& target, int low_index, int high_index) {
    int pivot_value = target[high_index];
    int smaller_boundary = low_index - 1;

    for (int scan_idx = low_index; scan_idx < high_index; ++scan_idx) {
        if (target[scan_idx] < pivot_value) {
            ++smaller_boundary;
            std::swap(target[smaller_boundary], target[scan_idx]);
        }
    }

    std::swap(target[smaller_boundary + 1], target[high_index]);
    return smaller_boundary + 1;
}

void quick_sort_recursive(std::vector<int>& target, int low_index, int high_index) {
    if (low_index < high_index) {
        int partition_point = split_array(target, low_index, high_index);

        quick_sort_recursive(target, low_index, partition_point - 1);
        quick_sort_recursive(target, partition_point + 1, high_index);
    }
}

std::vector<int> quick_sort_algorithm(const std::vector<int>& source_data) {
    std::vector<int> working_copy = source_data;
    quick_sort_recursive(working_copy, 0, working_copy.size() - 1);
    return working_copy;
}

int main() {
    std::random_device random_source;
    std::mt19937 random_engine(random_source());
    std::uniform_int_distribution<int> value_distribution(1, 1e6);
    std::vector<int> original_vector(100000);

    for (int& element : original_vector) {
        element = value_distribution(random_engine);
    }

    auto start_bubble = high_resolution_clock::now();
    auto bubble_result = bubble_sort_algorithm(original_vector);
    auto end_bubble = high_resolution_clock::now();
    std::cout << "Bubble sort took " << duration_cast<milliseconds>(end_bubble - start_bubble).count() << " ms" << std::endl;

    auto start_insertion = high_resolution_clock::now();
    auto insertion_result = insertion_sort_algorithm(original_vector);
    auto end_insertion = high_resolution_clock::now();
    std::cout << "Insertion sort took " << duration_cast<milliseconds>(end_insertion - start_insertion).count() << " ms" << std::endl;

    auto start_merge = high_resolution_clock::now();
    auto merge_result = merge_sort_algorithm(original_vector);
    auto end_merge = high_resolution_clock::now();
    std::cout << "Merge sort took " << duration_cast<milliseconds>(end_merge - start_merge).count() << " ms" << std::endl;

    auto start_quick = high_resolution_clock::now();
    auto quick_result = quick_sort_algorithm(original_vector);
    auto end_quick = high_resolution_clock::now();
    std::cout << "Quick sort took " << duration_cast<milliseconds>(end_quick - start_quick).count() << " ms" << std::endl;

    auto std_copy = original_vector;
    auto start_std = high_resolution_clock::now();
    std::sort(std_copy.begin(), std_copy.end(), [](int first, int second) { return first < second; });
    auto end_std = high_resolution_clock::now();
    std::cout << "std::sort took " << duration_cast<milliseconds>(end_std - start_std).count() << " ms" << std::endl;

    return 0;
}
