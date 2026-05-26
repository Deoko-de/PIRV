#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <boost/thread.hpp>
#include <atomic>
#include <mutex>

enum ProtectionType {
    NONE = 1,
    ATOM = 2,
    GUARD = 3
};

int unprotected_balance = 0;
std::atomic<int> protected_balance(0);
int locked_balance = 0;
std::mutex balance_lock;

std::vector<int> operation_log;

void processOperations(int start_idx, int end_idx, ProtectionType mechanism)
{
    for (int pos = start_idx; pos < end_idx; ++pos)
    {
        int delta = operation_log[pos];

        switch (mechanism)
        {
            case NONE:
                unprotected_balance += delta;
                break;

            case ATOM:
                protected_balance += delta;
                break;

            case GUARD:
            {
                std::lock_guard<std::mutex> locker(balance_lock);
                locked_balance += delta;
                break;
            }
        }
    }
}

void executeExperiment(int worker_count, int ops_per_worker, ProtectionType mechanism, long long correct_total)
{
    std::vector<boost::thread> worker_pool;
    
    int total_ops = worker_count * ops_per_worker;
    int chunk_size = total_ops / worker_count;
    
    auto timer_start = std::chrono::high_resolution_clock::now();
    
    for (int idx = 0; idx < worker_count; ++idx)
    {
        int segment_start = idx * chunk_size;
        int segment_end;
        
        if (idx == worker_count - 1)
        {
            segment_end = total_ops;
        }
        else
        {
            segment_end = segment_start + chunk_size;
        }
        
        worker_pool.emplace_back(processOperations, segment_start, segment_end, mechanism);
    }
    
    for (auto& th : worker_pool)
        th.join();
    
    auto timer_end = std::chrono::high_resolution_clock::now();
    
    double elapsed = std::chrono::duration<double>(timer_end - timer_start).count();
    
    long long final_value = 0;
    
    switch (mechanism)
    {
        case NONE:
            final_value = unprotected_balance;
            std::cout << "Mode: No synchronization\n";
            break;
            
        case ATOM:
            final_value = protected_balance;
            std::cout << "Mode: std::atomic<int>\n";
            break;
            
        case GUARD:
            final_value = locked_balance;
            std::cout << "Mode: std::mutex with lock_guard\n";
            break;
    }
    
    std::cout << "Execution time: " << elapsed << " seconds\n";
    std::cout << "Final balance: " << final_value << "\n";
    
    if (final_value == correct_total)
        std::cout << "Result is CORRECT\n\n";
    else
        std::cout << "Result is INCORRECT (expected " << correct_total << ")\n\n";
}

int main()
{
    int thread_count;
    int transactions_per_client;
    
    std::cout << "Enter number of clients (threads): ";
    std::cin >> thread_count;
    
    std::cout << "Enter number of transactions per client: ";
    std::cin >> transactions_per_client;
    
    int total_ops = thread_count * transactions_per_client;
    
    std::mt19937 rng(42);
    std::uniform_int_distribution<> range(-100, 100);
    
    operation_log.resize(total_ops);
    
    for (int i = 0; i < total_ops; ++i)
        operation_log[i] = range(rng);
    
    long long expected_total = 0;
    for (int amount : operation_log)
        expected_total += amount;
    
    std::cout << "\nExpected balance: " << expected_total << "\n\n";
    
    executeExperiment(thread_count, transactions_per_client, NONE, expected_total);
    executeExperiment(thread_count, transactions_per_client, ATOM, expected_total);
    executeExperiment(thread_count, transactions_per_client, GUARD, expected_total);
    
    return 0;
}
