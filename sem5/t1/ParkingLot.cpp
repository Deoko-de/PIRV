#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <condition_variable>
#include <chrono>

using namespace std;

class ParkingLot
{
private:

    int total_spaces;
    int used_spaces = 0;

    counting_semaphore<> space_semaphore;

    mutex operation_lock;
    condition_variable vip_condition;

    int vip_queue_count = 0;

public:

    ParkingLot(int space_count)
        : total_spaces(space_count),
          space_semaphore(space_count)
    {}

    void enter_parking(bool has_vip_priority, int wait_timeout_ms)
    {
        thread::id current_tid = this_thread::get_id();

        bool acquired = space_semaphore.try_acquire_for(
            chrono::milliseconds(wait_timeout_ms)
        );

        if(!acquired)
        {
            lock_guard<mutex> guard(operation_lock);
            if(has_vip_priority)
                vip_queue_count--;
            cout << "Thread " << current_tid
                 << " VIP:" << has_vip_priority
                 << " exceeded timeout\n";
            return;
        }

        unique_lock<mutex> locker(operation_lock);

        if(has_vip_priority)
            vip_queue_count++;

        if(!has_vip_priority)
        {
            vip_condition.wait(locker, [&]{
                return vip_queue_count == 0;
            });
        }

        if(has_vip_priority)
            vip_queue_count--;

        used_spaces++;

        cout << "Thread " << current_tid
             << " " << (has_vip_priority ? "VIP" : "Regular")
             << " parked. Occupied: "
             << used_spaces
             << " Free: "
             << total_spaces - used_spaces
             << endl;

        locker.unlock();

        this_thread::sleep_for(chrono::milliseconds(500));
        exit_parking();
    }

    void exit_parking()
    {
        {
            lock_guard<mutex> guard(operation_lock);

            used_spaces--;

            cout << "Thread "
                 << this_thread::get_id()
                 << " departed. Occupied: "
                 << used_spaces
                 << " Free: "
                 << total_spaces - used_spaces
                 << endl;
        }

        space_semaphore.release();

        vip_condition.notify_all();
    }

    void adjust_capacity(int new_space_count)
    {
        lock_guard<mutex> guard(operation_lock);

        int space_difference = new_space_count - total_spaces;
        total_spaces = new_space_count;

        if(space_difference > 0)
        {
            for(int i = 0; i < space_difference; ++i)
                space_semaphore.release();
        }

        cout << "Parking capacity changed to "
             << total_spaces << endl;
    }
};

void vehicle_behavior(ParkingLot& parking_area, bool has_vip_status)
{
    parking_area.enter_parking(has_vip_status, 1000);

    this_thread::yield();
}
