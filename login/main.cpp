#include <fb/login/context.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/leak.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/loader.h>
#ifndef _WIN32
#include <execinfo.h>
#else
#include "resource.h"
#endif

using namespace fb;

int main(int argc, char** argv)
{
    // Initialize config system
    if (!fb::init_config(argc, argv))
        return -1;

    try
    {
        //_CrtSetBreakAlloc(80);

#ifdef _WIN32
        ::set_console_icon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
#endif

        auto io_context = boost::asio::io_context{};
        auto context    = std::make_shared<fb::login::context>(io_context, fb::config<uint16_t>("port"));
        auto signals    = boost::asio::signal_set(io_context, SIGINT, SIGTERM);
        signals.async_wait([&context](const boost::system::error_code& ec, int signal_number) {
            context->exit();
        });
        fb::model::loader(context->model).run();
        context->run();
    }
    catch (std::exception& e)
    {
        std::cerr << "unhandled exception catched in main : " << e.what() << std::endl;
    }

    // Clean up
    return 0;
}