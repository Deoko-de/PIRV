#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <sstream>

template<typename T>
class Logger {

private:
    std::ofstream output_file;
    std::mutex write_lock;

    inline std::string convert_to_string(const T& data) {
        std::stringstream converter;
        converter << data;
        return converter.str();
    }

public:

    Logger(const std::string& file_path) {
        output_file.open(file_path);
    }

    ~Logger() {
        if (output_file.is_open())
            output_file.close();
    }

    void record(const T& content) {

        std::lock_guard<std::mutex> guard(write_lock);

        std::string formatted_message = convert_to_string(content);

        std::cout
            << "Thread "
            << std::this_thread::get_id()
            << ": "
            << formatted_message
            << std::endl;

        output_file
            << "Thread "
            << std::this_thread::get_id()
            << ": "
            << formatted_message
            << std::endl;
    }
};
