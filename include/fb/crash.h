#ifndef __FB_CRASH_H__
#define __FB_CRASH_H__

#include <atomic>
#include <cstddef>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <fb/windows.h>
#endif

namespace fb {

class crash
{
private:
    inline static char        _header[256]{};
    inline static size_t      _header_len = 0;
    inline static const char  _end[]      = "*** END CRASH ***\n";
    inline static std::mutex  _rotate_mutex;
    inline static std::string _kind;
    inline static std::string _path;
#ifdef _WIN32
    inline static std::atomic<HANDLE> _file          = INVALID_HANDLE_VALUE;
    inline static HANDLE              _previous_file = INVALID_HANDLE_VALUE;
    inline static HANDLE              _err           = INVALID_HANDLE_VALUE;
#else
    inline static std::atomic<int> _file_fd     = -1;
    inline static int              _previous_fd = -1;
    inline static char             _exe_path[512]{};
#endif

public:
    static void install(const char* service);
    static void rotate();

private:
    static void open_daily();
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
