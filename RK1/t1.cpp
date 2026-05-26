#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>

std::mutex m;
std::condition_variable c;
std::atomic<int> current_phase(0);

void execute_phase(int phase_number) {
    std::unique_lock<std::mutex> lock(m);
    
    c.wait(lock, [&]() { return phase_number == current_phase; });
    
    std::cout << "Running phase " << phase_number << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Phase " << phase_number << " finished" << std::endl;
    
    current_phase.fetch_add(1);
    lock.unlock();
    c.notify_all();
}

int main() {
    std::thread worker1(execute_phase, 0);
    std::thread worker2(execute_phase, 1);
    std::thread worker3(execute_phase, 2);
    std::thread worker4(execute_phase, 3);
    
    worker1.join();
    worker2.join();
    worker3.join();
    worker4.join();
    
    return 0;
}
