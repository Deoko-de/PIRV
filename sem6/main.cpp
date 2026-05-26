#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>

const char* MEMORY_BLOCK_NAME = "/shared_memory_block";
const char* BARRIER_SEMAPHORE = "/sync_semaphore";
const int TOTAL_WORKERS = 4;

struct SharedMemoryBlock {
    int values[TOTAL_WORKERS];
};

int main() {
    int memory_descriptor = shm_open(MEMORY_BLOCK_NAME, O_CREAT | O_RDWR, 0666);
    if (memory_descriptor == -1) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(memory_descriptor, sizeof(SharedMemoryBlock)) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    SharedMemoryBlock* shared_region = (SharedMemoryBlock*)mmap(
        nullptr, sizeof(SharedMemoryBlock), PROT_READ | PROT_WRITE, MAP_SHARED, memory_descriptor, 0
    );

    if (shared_region == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    for (int idx = 0; idx < TOTAL_WORKERS; ++idx) {
        shared_region->values[idx] = 0;
    }

    sem_t* sync_semaphore = sem_open(BARRIER_SEMAPHORE, O_CREAT, 0666, 0);
    if (sync_semaphore == SEM_FAILED) {
        perror("sem_open failed");
        return 1;
    }

    std::cout << "Master process launched. Awaiting worker processes to write data..." << std::endl;

    for (int worker_id = 0; worker_id < TOTAL_WORKERS; ++worker_id) {
        pid_t process_id = fork();

        if (process_id < 0) {
            perror("fork failed");
            return 1;
        }

        if (process_id == 0) {
            std::cout << "Worker process " << worker_id << " (PID: " << getpid() << ") started." << std::endl;
        }

        sleep(worker_id + 1);

        shared_region->values[worker_id] = (worker_id + 1) * 10;
        std::cout << "Worker process " << worker_id << " (PID: " << getpid() << ") wrote value: " << shared_region->values[worker_id] << std::endl;

        std::cout << "Worker process " << worker_id << " (PID: " << getpid() << ") reached synchronization point." << std::endl;
        sem_post(sync_semaphore);

        munmap(shared_region, sizeof(SharedMemoryBlock));
        close(memory_descriptor);
        sem_close(sync_semaphore);
    }

    std::cout << "Master process waiting for workers to reach synchronization barrier..." << std::endl;
    for (int idx = 0; idx < TOTAL_WORKERS; ++idx) {
        sem_wait(sync_semaphore);
    }

    std::cout << "All workers reached the barrier. Master process reading collected data..." << std::endl;
    for (int worker_id = 0; worker_id < TOTAL_WORKERS; ++worker_id) {
        std::cout << "Data from worker " << worker_id << ": " << shared_region->values[worker_id] << std::endl;
    }

    for (int idx = 0; idx < TOTAL_WORKERS; ++idx) {
        wait(nullptr);
    }

    munmap(shared_region, sizeof(SharedMemoryBlock));
    close(memory_descriptor);
    shm_unlink(MEMORY_BLOCK_NAME);

    sem_close(sync_semaphore);
    sem_unlink(BARRIER_SEMAPHORE);

    std::cout << "Master process terminated successfully." << std::endl;
    return 0;
}
