#ifndef TASK_H
#define TASK_H

class Task {
private:
    int value;
    int priority;
    int duration_ms;
    int steps;
    int currentStep;

public:
    Task(int value, int priority, int duration, int steps) : value(value), priority(priority), duration_ms(duration), steps(steps), currentStep(0) {}

    int result() {
        return static_cast<int>(value * value);
    }

    bool isFinished() {
        return currentStep >= steps;
    }

    int getValue() {
        return value;
    }

    int getPriority() {
        return priority;
    }

    int getDuration() {
        return duration_ms;
    }

    int getSteps() {
        return steps;
    }

    int getCurrentStep() {
        return currentStep;
    }

    void increaseCurrentStep() {
        currentStep++;
    }
};

#endif
