#include "logger.h"

#include <iostream>

Mutex Logger::mutex_;

Logger::Logger(const std::string &prefix)
    : prefix_(prefix)
{
}

Logger::~Logger()
{
    Mutex::ScopedLock lock(mutex_);
    std::cout << prefix_ << stream_.str() << std::endl;
}
