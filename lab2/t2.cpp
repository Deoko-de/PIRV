#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <random>
#include <chrono>
#include <semaphore>

using namespace std;

struct Vehicle {
    int serial;
    bool is_priority;
};

struct Junction {
    int index;

    queue<Vehicle> north_south;
    queue<Vehicle> east_west;

    mutex data_lock;

    int max_queue_length = 15;

    bool priority_ns = false;
    bool priority_ew = false;

    counting_semaphore<> traffic_semaphore{5};
};

vector<Junction> network(10);
mutex display_lock;

void handle_vehicle(Junction& junction, Vehicle vehicle, int junction_id, string heading) {

    junction.traffic_semaphore.acquire();

    {
        lock_guard<mutex> out(display_lock);
        cout << "Intersection " << junction_id
             << " allowing vehicle " << vehicle.serial
             << " (" << heading << ")"
             << (vehicle.is_priority ? " (EMERGENCY)" : "") << endl;
    }

    this_thread::sleep_for(chrono::milliseconds(600));

    {
        lock_guard<mutex> out(display_lock);
        cout << "Vehicle " << vehicle.serial
             << " passed intersection " << junction_id << endl;
    }

    junction.traffic_semaphore.release();
}

void spawn_vehicles() {
    random_device rand_dev;
    mt19937 rand_gen(rand_dev());
    uniform_int_distribution<> junction_selector(0, 9);
    uniform_int_distribution<> priority_chance(0, 25);

    int vehicle_counter = 0;

    while (true) {
        int selected_junction = junction_selector(rand_gen);

        Vehicle new_vehicle;
        new_vehicle.serial = ++vehicle_counter;
        new_vehicle.is_priority = (priority_chance(rand_gen) == 0);

        {
            lock_guard<mutex> lock(network[selected_junction].data_lock);
            uniform_int_distribution<> direction_selector(0, 9);

            if (direction_selector(rand_gen) < 7) {
                network[selected_junction].north_south.push(new_vehicle);
                if (new_vehicle.is_priority) network[selected_junction].priority_ns = true;
            } else {
                network[selected_junction].east_west.push(new_vehicle);
                if (new_vehicle.is_priority) network[selected_junction].priority_ew = true;
            }
        }

        {
            lock_guard<mutex> out(display_lock);
            cout << "Vehicle " << new_vehicle.serial
                 << " arrived at intersection " << selected_junction
                 << (new_vehicle.is_priority ? " (EMERGENCY)" : "") << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(30));
    }
}

void control_signals(int junction_index) {
    while (true) {
        Junction& current = network[junction_index];

        int ns_queue_size, ew_queue_size;

        {
            lock_guard<mutex> lock(current.data_lock);
            ns_queue_size = current.north_south.size();
            ew_queue_size = current.east_west.size();
        }

        int total_waiting = ns_queue_size + ew_queue_size;
        double congestion = (double)total_waiting / current.max_queue_length;

        int green_duration = 2;
        string active_route = "NS";

        if (current.priority_ns || current.priority_ew) {
            green_duration = 6;
            active_route = current.priority_ns ? "NS" : "EW";

            lock_guard<mutex> out(display_lock);
            cout << "EMERGENCY VEHICLE at intersection " << junction_index << endl;
        }
        if (abs(ns_queue_size - ew_queue_size) > 3) {
            green_duration = 7;
            active_route = (ns_queue_size > ew_queue_size) ? "NS" : "EW";

            lock_guard<mutex> out(display_lock);
            cout << "CRITICAL IMBALANCE at intersection " << junction_index << endl;
        }
        if (congestion > 0.7) {
            green_duration = 5;
            active_route = (ns_queue_size > ew_queue_size) ? "NS" : "EW";

            lock_guard<mutex> out(display_lock);
            cout << "OVERRIDDEN CONGESTION at intersection " << junction_index << endl;
        }
        else {
            static int cycle_counter = 0;
            active_route = (++cycle_counter % 2 == 0) ? "NS" : "EW";
        }

        for (int second = 0; second < green_duration; second++) {
            Vehicle next_vehicle;
            bool vehicle_exists = false;

            {
                lock_guard<mutex> lock(current.data_lock);

                if (active_route == "NS" && !current.north_south.empty()) {
                    next_vehicle = current.north_south.front();
                    current.north_south.pop();
                    vehicle_exists = true;
                    if (next_vehicle.is_priority) current.priority_ns = false;
                }
                else if (active_route == "EW" && !current.east_west.empty()) {
                    next_vehicle = current.east_west.front();
                    current.east_west.pop();
                    vehicle_exists = true;
                    if (next_vehicle.is_priority) current.priority_ew = false;
                }
            }

            if (vehicle_exists) {
                thread(handle_vehicle, ref(current), next_vehicle, junction_index, active_route).detach();
            }
        }

        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main() {

    vector<thread> signal_controllers;

    for (int idx = 0; idx < 10; idx++) {
        network[idx].index = idx;
        signal_controllers.emplace_back(control_signals, idx);
    }

    thread vehicle_spawner(spawn_vehicles);

    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
