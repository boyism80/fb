#include "resource.h"
#include <context.h>
#include <fb/socket.h>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <fb/leak.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/protocol/flatbuffer/protocol.h>
#ifndef _WIN32
#include <execinfo.h>
#endif

using namespace fb;

int main(int argc, const char** argv)
{
    try
    {
        //_CrtSetBreakAlloc(165);

#ifdef _WIN32
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
#endif

        auto height = 9;
        console::box(console::width() - 1, height);

        auto header = "The Kingdom of the wind [GATEWAY]";
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
        boost::asio::io_context io_context;
        auto context = std::make_unique<fb::gateway::context>(io_context, fb::config<uint16_t>("port"));
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