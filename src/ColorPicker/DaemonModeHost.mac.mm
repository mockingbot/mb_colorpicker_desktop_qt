#include "DaemonModeHost.hxx"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <type_traits>

int32_t WaitForGetFired()
{
    static auto pipe_handle = fileno(stdin);

    using namespace std::placeholders;  // for _1, _2, _3...

    using T = int32_t;

    T value = 0;
    auto value_buffer = reinterpret_cast<char*>(&value);

    auto read_pipe = std::bind(::read, pipe_handle, _1, _2);
    if( read_pipe(value_buffer, sizeof(T)) != sizeof(T) )
    {
        fprintf(stderr, "%s ::red failed\n", __CURRENT_FUNCTION_NAME__);
        fflush(stderr);
        throw std::runtime_error(std::string(__CURRENT_FUNCTION_NAME__)+" ::read failed");
    }

    return value;
}
