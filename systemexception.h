#ifndef SYSTEMEXCEPTION_H
#define SYSTEMEXCEPTION_H

#include <exception>

class SystemException : public std::exception
{
public:
    SystemException(int ec, const char * what)
        : ec_(ec)
        , what_(what)
    {
    }

    int ec() const { return ec_; }
    const char * what() const noexcept { return what_; }

private:
    int ec_;
    const char * what_;
};

void throw_error(int ec, const char * what);
//#define REACTOR_DEBUG

#endif // SYSTEMEXCEPTION_H
