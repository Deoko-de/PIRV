#include <iostream>
#include <thread>
#include <chrono>

#include "Bank.cpp"

int main() {

    FinancialInstitution<int> banking_system(5, 1000);

    std::cout << "Initial total balance = " << banking_system.compute_total_reserves() << std::endl;

    for (int operation_idx = 0; operation_idx < 10; ++operation_idx) {

        int sender_account = operation_idx % 5;
        int receiver_account = (operation_idx + 1) % 5;

        banking_system.perform_transfer(sender_account, receiver_account, 100);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Final total balance = " << banking_system.compute_total_reserves() << std::endl;

    return 0;
}
