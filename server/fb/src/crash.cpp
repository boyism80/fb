#include <fb/crash.h>
#include <fb/config.h>
#include <fb/encoding.h>
#include <fb/logger.h>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#endif

void fb::crash::write_bytes(const void* data, size_t size)
{
#ifdef _WIN32
    DWORD written = 0;
    if (_err != INVALID_HANDLE_VALUE)
        WriteFile(_err, data, static_cast<DWORD>(size), &written, nullptr);
    if (_file != INVALID_HANDLE_VALUE)
        WriteFile(_file, data, static_cast<DWORD>(size), &written, nullptr);
#else
    if (size == 0)
        return;
    ::write(STDERR_FILENO, data, size);
    if (_file_fd >= 0)
        ::write(_file_fd, data, size);
#endif
}

#ifdef _WIN32
void fb::crash::write_dump(EXCEPTION_POINTERS* info)
{
    HANDLE  process = GetCurrentProcess();
    HANDLE  thread  = GetCurrentThread();
    CONTEXT ctx{};
    if (info != nullptr && info->ContextRecord != nullptr)
        ctx = *info->ContextRecord;
    else
        RtlCaptureContext(&ctx);

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(process, nullptr, TRUE);

    STACKFRAME64 frame{};
#ifdef _WIN64
    auto machine           = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset    = ctx.Rip;
    frame.AddrPC.Mode      = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Rbp;
    frame.AddrFrame.Mode   = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Rsp;
    frame.AddrStack.Mode   = AddrModeFlat;
#else
    auto machine           = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset    = ctx.Eip;
    frame.AddrPC.Mode      = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Ebp;
    frame.AddrFrame.Mode   = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Esp;
    frame.AddrStack.Mode   = AddrModeFlat;
#endif

    for (int i = 0; i < 64; i++)
    {
        if (StackWalk64(machine,
                        process,
                        thread,
                        &frame,
                        &ctx,
                        nullptr,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        nullptr) == FALSE)
            break;
        if (frame.AddrPC.Offset == 0)
            break;

        char                      line[1024];
        alignas(SYMBOL_INFO) char symbol_storage[sizeof(SYMBOL_INFO) + 256];
        auto                      symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_storage);
        std::memset(symbol_storage, 0, sizeof(symbol_storage));
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = 255;
        DWORD64 displacement = 0;
        int     n            = 0;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol))
        {
            IMAGEHLP_LINE64 src{};
            DWORD           line_disp = 0;
            src.SizeOfStruct          = sizeof(src);
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &line_disp, &src) && src.FileName != nullptr)
            {
                n = std::snprintf(line,
                                  sizeof(line),
                                  "#%d %s +0x%llx at %s:%lu\n",
                                  i,
                                  symbol->Name,
                                  static_cast<unsigned long long>(displacement),
                                  src.FileName,
                                  src.LineNumber);
            }
            else
            {
                n = std::snprintf(line,
                                  sizeof(line),
                                  "#%d %s +0x%llx [0x%llx]\n",
                                  i,
                                  symbol->Name,
                                  static_cast<unsigned long long>(displacement),
                                  static_cast<unsigned long long>(frame.AddrPC.Offset));
            }
        }
        else
        {
            n = std::snprintf(line,
                              sizeof(line),
                              "#%d 0x%llx\n",
                              i,
                              static_cast<unsigned long long>(frame.AddrPC.Offset));
        }

        if (n > 0)
        {
            auto size = static_cast<size_t>(n);
            if (size >= sizeof(line))
                size = sizeof(line) - 1;
            write_bytes(line, size);
        }
    }
}

void fb::crash::dump(const char* kind, size_t kind_len, EXCEPTION_POINTERS* info)
{
    write_bytes(_header, _header_len);
    write_bytes(kind, kind_len);
    write_dump(info);
    write_bytes(_end, sizeof(_end) - 1);
    if (_file != INVALID_HANDLE_VALUE)
        FlushFileBuffers(_file);
    if (_err != INVALID_HANDLE_VALUE)
        FlushFileBuffers(_err);
}
#else
void fb::crash::write_dump()
{
    void* frames[64] = {};
    auto  count      = backtrace(frames, 64);
    if (count <= 0)
        return;
    backtrace_symbols_fd(frames, count, STDERR_FILENO);
    if (_file_fd >= 0)
        backtrace_symbols_fd(frames, count, _file_fd);
}

void fb::crash::dump(const char* kind, size_t kind_len)
{
    write_bytes(_header, _header_len);
    write_bytes(kind, kind_len);
    write_dump();
    write_bytes(_end, sizeof(_end) - 1);
    if (_file_fd >= 0)
        ::fsync(_file_fd);
    ::fsync(STDERR_FILENO);
}
#endif

#ifndef _WIN32
void fb::crash::on_signal(int sig)
{
    if (sig == SIGSEGV)
        dump("signal=SIGSEGV\n", sizeof("signal=SIGSEGV\n") - 1);
    else if (sig == SIGABRT)
        dump("signal=SIGABRT\n", sizeof("signal=SIGABRT\n") - 1);
    else if (sig == SIGBUS)
        dump("signal=SIGBUS\n", sizeof("signal=SIGBUS\n") - 1);
    else if (sig == SIGFPE)
        dump("signal=SIGFPE\n", sizeof("signal=SIGFPE\n") - 1);
    else if (sig == SIGILL)
        dump("signal=SIGILL\n", sizeof("signal=SIGILL\n") - 1);
    else
        dump("signal=unknown\n", sizeof("signal=unknown\n") - 1);
    std::_Exit(128 + sig);
}
#else
LONG WINAPI fb::crash::on_unhandled(EXCEPTION_POINTERS* info)
{
    char kind[64];
    auto code = 0ul;
    if (info != nullptr && info->ExceptionRecord != nullptr)
        code = static_cast<unsigned long>(info->ExceptionRecord->ExceptionCode);

    auto n = std::snprintf(kind, sizeof(kind), "unhandled_exception code=0x%08lX\n", code);
    if (n < 0)
        dump("unhandled_exception\n", sizeof("unhandled_exception\n") - 1, info);
    else
        dump(kind, static_cast<size_t>(n), info);
    return EXCEPTION_CONTINUE_SEARCH;
}

void fb::crash::on_abrt(int)
{
    dump("signal=SIGABRT\n", sizeof("signal=SIGABRT\n") - 1);
    std::_Exit(3);
}
#endif

void fb::crash::on_terminate()
{
    auto what = std::string{};
    try
    {
        auto e = std::current_exception();
        if (e)
            std::rethrow_exception(e);
    }
    catch (const std::exception& e)
    {
        what = e.what();
    }
    catch (...)
    {
        what = "unknown";
    }

    fb::logger::fatal("terminate: {}", what);

    auto kind = std::format("terminate: {}\n", what);
#ifdef _WIN32
    kind = fb::utf8(kind);
#endif
    dump(kind.c_str(), kind.size());
    std::_Exit(1);
}

void fb::crash::install(const char* service)
{
    static std::once_flag flag;
    std::call_once(flag, [service] {
        auto kind = service != nullptr ? service : "unknown";
        auto name = fb::config<std::string>("name", std::string{kind});
        if (name.empty())
            name = kind;
        auto file = fb::logger::daily_path(kind);
#ifdef _WIN32
        auto pid = static_cast<unsigned long>(GetCurrentProcessId());
        _err     = GetStdHandle(STD_ERROR_HANDLE);
        try
        {
            std::filesystem::create_directories("logs");
        }
        catch (...)
        { }
        _file = CreateFileA(file.c_str(),
                            FILE_APPEND_DATA,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            nullptr);
#else
        auto pid = static_cast<long>(::getpid());
        try
        {
            std::filesystem::create_directories("logs");
        }
        catch (...)
        {
        }
        _file_fd = ::open(file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
#endif
        _header_len = static_cast<size_t>(std::snprintf(_header,
                                                        sizeof(_header),
                                                        "*** FB CRASH ***\nservice=%s name=%s pid=%lu\n",
                                                        kind,
                                                        name.c_str(),
                                                        static_cast<unsigned long>(pid)));
        if (_header_len >= sizeof(_header))
            _header_len = sizeof(_header) - 1;

#ifndef _WIN32
        struct sigaction sa{};
        sa.sa_handler = on_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESETHAND;
        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGBUS, &sa, nullptr);
        sigaction(SIGFPE, &sa, nullptr);
        sigaction(SIGILL, &sa, nullptr);
#else
        SetUnhandledExceptionFilter(on_unhandled);
        std::signal(SIGABRT, on_abrt);
#endif
        std::set_terminate(on_terminate);
    });
}
