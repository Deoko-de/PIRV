#ifndef VIRTUALTHREAD_H
#define VIRTUALTHREAD_H
#include <iostream>

class VirtualThread {
private:
    int values[4] = {5, 7, 10, 12};
    int thread_id;
    short position_index;
    
    long long compute_factorial(int number) {
        if (number == 1 || number == 0) return 1;
        
        return number * compute_factorial(number - 1);
    }
public:
    VirtualThread(int id) : thread_id(id), position_index(0) {}

    bool hasTasks() {
        return position_index < 4;
    }

    void increaseCurrentIndex() {
        position_index++;
    }

    void run() {
        int current_value = values[position_index];
        long long factorial_result = compute_factorial(current_value);

        std::cout << "Virtual thread " << thread_id << " computes " << current_value << "! = " << factorial_result << std::endl;
        increaseCurrentIndex();
    }  
};

#endif
