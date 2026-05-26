#include <vector>
#include <thread>
#include <iostream>

#include "Account.cpp"

template<typename NumericType>
class FinancialInstitution {

private:

    std::vector<BankAccount<NumericType>> client_accounts;

    inline void execute_transfer(int source_index, int destination_index, NumericType transaction_amount) {

        client_accounts[source_index].withdraw_funds(transaction_amount);
        client_accounts[destination_index].add_funds(transaction_amount);

        std::cout
            << "Worker "
            << std::this_thread::get_id()
            << " transfers "
            << transaction_amount
            << " from account "
            << source_index
            << " to account "
            << destination_index
            << std::endl;
    }

public:

    FinancialInstitution(int account_count, NumericType opening_balance) {

        for (int idx = 0; idx < account_count; ++idx) {
            client_accounts.emplace_back(opening_balance);
        }
    }

    void perform_transfer(int sender, int receiver, NumericType amount) {

        std::thread([=]() {

            execute_transfer(sender, receiver, amount);

        }).detach();
    }

    NumericType compute_total_reserves() {

        NumericType accumulated = 0;

        for (auto& account_ref : client_accounts) {
            accumulated += account_ref.check_balance();
        }

        return accumulated;
    }
};
