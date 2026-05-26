#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

std::mutex m;
std::condition_variable c;
std::atomic<int> current_phase(0);

void execute_phase(int phase_number, int total_phases) {
    std::unique_lock<std::mutex> lock(m);
    
    c.wait(lock, [&]() { return phase_number == current_phase; });
    
    std::cout << "Running phase " << phase_number << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Phase " << phase_number << " finished" << std::endl;
    
    current_phase.fetch_add(1);
    lock.unlock();
    
    if (phase_number + 1 < total_phases) {
        c.notify_all();
    }
}

int main() {
    const int TOTAL_PHASES = 4;
    std::vector<std::thread> workers;
    
    for (int i = 0; i < TOTAL_PHASES; ++i) {
        workers.emplace_back(execute_phase, i, TOTAL_PHASES);
    }
    
    for (auto& w : workers) {
        w.join();
    }
    
    return 0;
}
