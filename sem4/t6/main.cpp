#include "MatrixProcessor.cpp"
#include <vector>
#include <iostream>

int main() {
    std::vector<std::vector<int>> source_matrix = {
        {-1, 2, 3},
        {4, -5, 6},
        {7, -8, 9},
        {10, 11, -12}
    };

    MatrixProcessor<int> matrix_worker(source_matrix, 3);

    matrix_worker.execute_transformation([](int element) { return element * 5; });

    matrix_worker.display_matrix();

    return 0;
}
