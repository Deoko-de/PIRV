#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <random>
#include <mutex>
#include <chrono>
#include <memory>
#include <atomic>

struct Job {
    int number;
    int importance;
    int processing_time;
};

struct PriorityOrder {
    bool operator()(const Job& first, const Job& second) {
        return first.importance > second.importance;
    }
};

struct WorkerNode {
    int identifier;
    std::priority_queue<Job, std::vector<Job>, PriorityOrder> pending_jobs;
    std::mutex node_lock;

    WorkerNode() : identifier(0) {}
};

std::vector<std::unique_ptr<WorkerNode>> nodes;
std::mutex global_node_lock;
std::mutex console_lock;
std::atomic<int> finished_jobs{0};

int job_counter = 0;

void register_new_node() {
    std::lock_guard<std::mutex> lock(global_node_lock);

    nodes.push_back(std::make_unique<WorkerNode>());
    nodes.back()->identifier = nodes.size();

    std::lock_guard<std::mutex> out(console_lock);
    std::cout << "New server node added! ID: "
              << nodes.back()->identifier << "\n";
}

void dispatch_jobs(int amount) {
    std::random_device random_source;
    std::mt19937 random_generator(random_source());
    std::uniform_int_distribution<> priority_range(1, 3);
    std::uniform_int_distribution<> duration_range(1, 5);

    for (int iteration = 0; iteration < amount; iteration++) {
        Job current_job = {++job_counter, priority_range(random_generator), duration_range(random_generator)};

        int target_node;

        {
            std::lock_guard<std::mutex> lock(global_node_lock);
            std::uniform_int_distribution<> node_distribution(0, nodes.size() - 1);
            target_node = node_distribution(random_generator);

            std::lock_guard<std::mutex> node_lock(nodes[target_node]->node_lock);
            nodes[target_node]->pending_jobs.push(current_job);
        }

        std::lock_guard<std::mutex> out(console_lock);
        std::cout << "Job " << current_job.number << " assigned to node "
                  << nodes[target_node]->identifier
                  << " (priority " << current_job.importance << ")\n";
    }
}

void redistribute_workload() {
    std::lock_guard<std::mutex> lock(global_node_lock);

    for (int first = 0; first < nodes.size(); first++) {
        for (int second = 0; second < nodes.size(); second++) {
            if (first == second) continue;

            std::scoped_lock dual_lock(nodes[first]->node_lock, nodes[second]->node_lock);

            if (nodes[first]->pending_jobs.size() > 3 &&
                nodes[second]->pending_jobs.size() < 2) {

                Job transferred = nodes[first]->pending_jobs.top();
                nodes[first]->pending_jobs.pop();
                nodes[second]->pending_jobs.push(transferred);

                std::lock_guard<std::mutex> out(console_lock);
                std::cout << "Job " << transferred.number
                          << " migrated from node "
                          << nodes[first]->identifier
                          << " to node "
                          << nodes[second]->identifier << "\n";

                break;
            }
        }
    }
}

void node_processor(int position) {
    while (true) {
        Job current_job;
        bool job_available = false;
        WorkerNode* active_node;

        {
            std::lock_guard<std::mutex> lock(global_node_lock);
            if (position >= nodes.size()) return;
            active_node = nodes[position].get();
        }

        {
            std::lock_guard<std::mutex> lock(active_node->node_lock);
            if (!active_node->pending_jobs.empty()) {
                current_job = active_node->pending_jobs.top();
                active_node->pending_jobs.pop();
                job_available = true;
            }
        }

        if (!job_available) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        {
            std::lock_guard<std::mutex> out(console_lock);
            std::cout << "Node " << active_node->identifier
                      << " processing job " << current_job.number
                      << " (priority " << current_job.importance << ")\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(current_job.processing_time));

        {
            std::lock_guard<std::mutex> out(console_lock);
            std::cout << "Job " << current_job.number
                      << " completed by node "
                      << active_node->identifier << "\n";
            finished_jobs++;
        }
    }
}

void system_supervisor() {
    while (true) {
        int total_pending = 0;
        int node_count = 0;

        {
            std::lock_guard<std::mutex> lock(global_node_lock);

            node_count = nodes.size();

            for (auto& worker : nodes) {
                std::lock_guard<std::mutex> lock_node(worker->node_lock);
                total_pending += worker->pending_jobs.size();
            }
        }

        if (node_count == 0) continue;

        double utilization = (double)total_pending / node_count;

        if (utilization > 0.8) {
            register_new_node();

            std::lock_guard<std::mutex> lock(global_node_lock);
            std::thread(node_processor, nodes.size() - 1).detach();
        }

        redistribute_workload();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    for (int initial = 0; initial < 5; initial++)
        register_new_node();

    for (int idx = 0; idx < nodes.size(); idx++) {
        std::thread(node_processor, idx).detach();
    }

    std::thread(system_supervisor).detach();

    dispatch_jobs(20);

    std::this_thread::sleep_for(std::chrono::seconds(20));
    std::cout << "Total jobs completed: " << finished_jobs << "\n";

    return 0;
}
