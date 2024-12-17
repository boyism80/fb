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
#else
#include "resource.h"
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

        auto io_context = boost::asio::io_context{};
        auto context    = std::make_unique<fb::gateway::context>(io_context, config<uint16_t>("port"));
        context->run();
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(std::format("unhandled exception catched in main : {}", e.what()));
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