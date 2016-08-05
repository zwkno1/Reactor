
#include "systemexception.h"

void throw_error(int ec, const char * what)
{
    if(ec)
        throw SystemException(ec, what);
}
