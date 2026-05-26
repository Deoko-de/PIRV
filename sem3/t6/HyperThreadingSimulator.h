#ifndef HYPERTHREADINGSIMULATOR_H
#define HYPERTHREADINGSIMULATOR_H
#include "VirtualThread.h"

class HyperThreadingSimulator {
private:
    VirtualThread logical_core_a;
    VirtualThread logical_core_b;
public:
    HyperThreadingSimulator() : logical_core_a(1), logical_core_b(2) {}

    void start_execution() {
        while (logical_core_a.hasTasks() || logical_core_b.hasTasks()) {
            if (logical_core_a.hasTasks()) {
                logical_core_a.run();
                logical_core_b.increaseCurrentIndex();
            }

            if (logical_core_b.hasTasks()) {
                logical_core_b.run();
                logical_core_a.increaseCurrentIndex();
            }
        }
    }
};

#endif
