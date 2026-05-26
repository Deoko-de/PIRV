#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <boost/thread.hpp>

void initializeMatrix(std::vector<std::vector<double>>& data, int size) {
    std::mt19937 generator(42);
    std::uniform_real_distribution<> distribution(0.0, 10.0);

    for (int r = 0; r < size; ++r)
        for (int c = 0; c < size; ++c)
            data[r][c] = distribution(generator);
}

void computeRow(const std::vector<std::vector<double>>& first, const std::vector<std::vector<double>>& second,
                std::vector<std::vector<double>>& result, int dimension, int rowIndex) {
    for (int col = 0; col < dimension; ++col) {
        result[rowIndex][col] = 0;
        for (int k = 0; k < dimension; ++k) {
            result[rowIndex][col] += first[rowIndex][k] * second[k][col];
        }
    }
}

void sequentialMultiply(const std::vector<std::vector<double>>& first, const std::vector<std::vector<double>>& second,
                         std::vector<std::vector<double>>& result, int dimension) {
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < dimension; ++k) {
                result[i][j] += first[i][k] * second[k][j];
            }
        }
    }
}

void parallelMultiply(const std::vector<std::vector<double>>& first, const std::vector<std::vector<double>>& second,
                      std::vector<std::vector<double>>& result, int dimension) {
    std::vector<boost::thread> workers;

    for (int row = 0; row < dimension; ++row) {
        workers.emplace_back(computeRow, std::cref(first), std::cref(second),
                             std::ref(result), dimension, row);
    }

    for (auto& worker : workers)
        worker.join();
}

int main() {
    int dimension = 500;

    std::vector<std::vector<double>> matrixA(dimension, std::vector<double>(dimension));
    std::vector<std::vector<double>> matrixB(dimension, std::vector<double>(dimension));
    std::vector<std::vector<double>> matrixC(dimension, std::vector<double>(dimension));

    initializeMatrix(matrixA, dimension);
    initializeMatrix(matrixB, dimension);

    auto startParallel = std::chrono::high_resolution_clock::now();

    parallelMultiply(matrixA, matrixB, matrixC, dimension);

    auto endParallel = std::chrono::high_resolution_clock::now();

    double parallelTime = std::chrono::duration<double>(endParallel - startParallel).count();

    std::cout << "Parallel execution time: "
              << parallelTime
              << " seconds\n";

    auto startSequential = std::chrono::high_resolution_clock::now();

    sequentialMultiply(matrixA, matrixB, matrixC, dimension);

    auto endSequential = std::chrono::high_resolution_clock::now();

    double sequentialTime = std::chrono::duration<double>(endSequential - startSequential).count();

    std::cout << "Sequential execution time: "
              << sequentialTime
              << " seconds\n";

    std::cout << "Parallel execution is " << sequentialTime / parallelTime << " times faster than sequential\n";

    return 0;
}
