#include "DaemonModeHost.hxx"


#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <type_traits>

int32_t WaitForGetFired()
{
    static auto pipe_handle = ::GetStdHandle(STD_INPUT_HANDLE);

    using namespace std::placeholders;  // for _1, _2, _3...

    using T = int32_t;

    T value = 0;
    auto value_buffer = reinterpret_cast<char*>(&value);
    DWORD read_size;

    auto read_pipe = std::bind(::ReadFile, pipe_handle, _1, _2, _3, _4);
    if( read_pipe(value_buffer, (DWORD)sizeof(T), &read_size, nullptr) != TRUE )
    {
        printf(__CURRENT_FUNCTION_NAME__ "::ReadFile Failed 1\n");
        throw std::runtime_error(__CURRENT_FUNCTION_NAME__ "::ReadFile Failed 1\n");
    }
    if( read_size != sizeof(T) )
    {
        printf(__CURRENT_FUNCTION_NAME__ "::ReadFile Failed 2\n");
        throw std::runtime_error(__CURRENT_FUNCTION_NAME__ "::ReadFile Failed 2\n");
    }

    return value;
}
