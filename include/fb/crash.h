#ifndef __FB_CRASH_H__
#define __FB_CRASH_H__

#include <cstddef>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fb {

class crash
{
private:
    inline static char       _header[256]{};
    inline static size_t     _header_len = 0;
    inline static const char _end[]      = "*** END CRASH ***\n";
#ifdef _WIN32
    inline static HANDLE _file = INVALID_HANDLE_VALUE;
    inline static HANDLE _err  = INVALID_HANDLE_VALUE;
#else
    inline static int _file_fd = -1;
#endif

public:
    static void install(const char* service);

private:
    static void write_bytes(const void* data, size_t size);
#ifdef _WIN32
    static void write_dump(EXCEPTION_POINTERS* info);
    static void dump(const char* kind, size_t kind_len, EXCEPTION_POINTERS* info = nullptr);
#else
    static void write_dump();
    static void dump(const char* kind, size_t kind_len);
#endif
#ifndef _WIN32
    static void on_signal(int sig);
#else
    static LONG WINAPI on_unhandled(EXCEPTION_POINTERS* info);
    static void        on_abrt(int sig);
#endif
    static void on_terminate();
};

} // namespace fb

#endif
