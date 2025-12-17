#pragma once

#include "npc.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) = 0;
};

class ConsoleObserver : public IObserver {
private:
    mutable std::mutex mutex;
public:
    void on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) override;
};

class FileObserver : public IObserver {
private:
    std::string filename;
    mutable std::mutex mutex;
public:
    FileObserver(const std::string& filename = "log.txt") : filename(filename) {}
    void on_kill(const std::shared_ptr<INpc>& killer, const std::shared_ptr<INpc>& victim) override;
};