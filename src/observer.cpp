#include "observer.h"
#include <ctime>

void ConsoleObserver::on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) {
    std::lock_guard<std::mutex> lock(mutex);
    if (killer && victim) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        tm = *std::localtime(&time);
#endif
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "[KILL] " << killer->get_name() << " killed " << victim->get_name() << std::endl;
    }
}

void FileObserver::on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) {
    std::lock_guard<std::mutex> lock(mutex);
    if (killer && victim) {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &time);
#else
            tm = *std::localtime(&time);
#endif
            file << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ");
            file << killer->get_name() << " killed " << victim->get_name() << "\n";
        }
    }
}