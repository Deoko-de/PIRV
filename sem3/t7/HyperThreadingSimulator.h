#ifndef HYPERTHREADINGSIMULATOR_H
#define HYPERTHREADINGSIMULATOR_H
#include "VirtualThread.h"

class HyperThreadingSimulator {
private:
    VirtualThread core_a;
    VirtualThread core_b;
public:
    HyperThreadingSimulator(const std::vector<std::string>& pending_operations) : core_a(1, pending_operations), core_b(2, pending_operations) {}

    void start_execution() {
        while (core_a.hasOperations() || core_b.hasOperations()) {
            if (core_a.hasOperations()) {
                if (!core_a.isBusy()) core_b.advanceOperationIndex();
                core_a.processNextOperation();
            }

            if (core_b.hasOperations()) {
                if (core_b.isBusy()) core_a.advanceOperationIndex();
                core_b.processNextOperation();
            }
        }
    }
};

#endif
