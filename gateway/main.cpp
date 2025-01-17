#include <fb/gateway/context.h>
#include <fb/socket.h>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <fb/leak.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/protocol/flatbuffer/protocol.h>
#ifdef _WIN32
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
    }

    // Clean up
    return 0;
}