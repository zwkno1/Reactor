#ifndef LOGGER_H
#define LOGGER_H

#include "mutex.h"
#include <sstream>

class Logger
{
public:
    ~Logger();

    static inline Logger debug() { return Logger("[debug] "); }
    static inline Logger info()  { return Logger("[info ] "); }
    static inline Logger warn()  { return Logger("[warn ] "); }
    static inline Logger error() { return Logger("[error] "); }

    template<class T>
    Logger & operator <<(T t)
    {
        stream_ << t;
        return *this;
    }

private:
    Logger(const std::string & prefix);
    Logger(Logger && other);

    std::stringstream stream_;
    std::string prefix_;
    static Mutex mutex_;
};

#endif // LOGGER_H
