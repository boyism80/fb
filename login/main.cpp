#include "resource.h"
#include <context.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/leak.h>
#include <fb/protocol/flatbuffer/protocol.h>
#ifndef _WIN32
#include <execinfo.h>
#endif

using namespace fb;

int main(int argc, const char** argv)
{
    try
    {
        //_CrtSetBreakAlloc(80);

#ifdef _WIN32
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        auto height = 9;
        console::box(console::width() - 1, height);

        auto header = "The Kingdom of the wind [LOGIN]";
        console::cursor((console::width() - 1 - strlen(header)) / 2, 3);
        console::render(header);

        auto github = "https://github.com/boyism80/fb";
        console::cursor(console::width() - 1 - strlen(github) - 3, 5);
        console::render(github);

        auto madeby = "made by cshyeon";
        console::cursor(console::width() - 1 - strlen(madeby) - 3, 6);
        console::render(madeby);
        console::position(0, height + 1);

        // Execute acceptor
        auto& config = fb::config::get();

        // Execute acceptor
        auto io_context = boost::asio::io_context{};
        auto context    = std::make_unique<fb::login::context>(io_context, config["port"].asInt());
        context->run();
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
#ifndef _WIN32
        void*  array[10];
        size_t size;
        size = backtrace(array, 10);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
    }

    // Clean up
    return 0;
}