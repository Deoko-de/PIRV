#ifndef VIRTUALTHREAD_H
#define VIRTUALTHREAD_H
#include "Task.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

class VirtualThread {
private:
    std::vector<Task> pending_jobs;

public: 
    void addTask(const Task& job) {
        pending_jobs.push_back(job);
    }

    bool hasTasks() {
        return !pending_jobs.empty();
    }

    void runStep(int processor_id) {
        if (hasTasks()) {
            auto highest_priority = std::max_element(pending_jobs.begin(), pending_jobs.end(),
            [](Task& first, Task& second) { return first.getPriority() < second.getPriority(); });
        
            Task& current_job = *highest_priority;

            current_job.increaseCurrentStep();

            std::cout << "Virtual thread " << processor_id << " executing step " << current_job.getCurrentStep() << "/" << current_job.getSteps() 
            << " of job " << current_job.getValue() << " with priority " << current_job.getPriority() << std::endl;
        
            int step_duration = current_job.getDuration() / current_job.getSteps();
            std::this_thread::sleep_for(std::chrono::milliseconds(step_duration));

            if (current_job.isFinished()) {
                std::cout << "Virtual thread " << processor_id << " completed job " << current_job.getValue()
                << ": result = " << current_job.result() << std::endl;

                pending_jobs.erase(highest_priority);
            }
        }
    }
};

#endif
