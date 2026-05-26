#ifndef VIRTUALTHREAD_H
#define VIRTUALTHREAD_H
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

class VirtualThread {
private:
    int thread_identifier;
    std::vector<std::string> operation_queue;
    int operation_position;
    bool is_active;

public: 
    VirtualThread(int identifier, const std::vector<std::string>& pending_operations) 
        : thread_identifier(identifier), operation_queue(pending_operations), operation_position(0), is_active(false) {}

    bool hasOperations() {
        return is_active || operation_position < operation_queue.size();
    }

    void advanceOperationIndex() {
        operation_position++;
    }

    void processNextOperation() {
        if (hasOperations()) {
            if (!is_active) {  
                std::cout << "Virtual thread " << thread_identifier << " started " << operation_queue[operation_position] << std::endl;
                is_active = true;
            } else {
                std::cout << "Virtual thread " << thread_identifier << " finished " << operation_queue[operation_position] << std::endl;
                is_active = false;
                advanceOperationIndex();
            }
        }
    }

    bool isBusy() {
        return is_active;
    }
};

#endif
