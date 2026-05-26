#include <iostream>
#include <thread>
#include <chrono>
#include <random>

#include "ParkingLot.cpp"

int main()
{
    ParkingLot parking_area(3);

    for(int vehicle_index = 0; vehicle_index < 10; ++vehicle_index)
    {
        thread([&parking_area, vehicle_index]
        {
            std::random_device random_source;
            std::mt19937 random_engine(random_source());
            std::uniform_int_distribution<> vip_distribution(0,1);

            bool vip_status = vip_distribution(random_engine);
            vehicle_behavior(parking_area, vip_status);
        }).detach();
    }

    this_thread::sleep_for(chrono::seconds(2));

    parking_area.adjust_capacity(5);

    this_thread::sleep_for(chrono::seconds(5));
}
