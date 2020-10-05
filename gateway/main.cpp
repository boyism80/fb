#include <stdio.h>
#include <iostream>
#include "resource.h"
#include "module/socket/socket.h"
#include "model/acceptor/acceptor.gateway.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "module/config/config.h"

fb::gateway::acceptor*       acceptor;

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(165);

    auto& c = console::get();
    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [GATEWAY]";
    c.puts(header, (c.width()-1 - strlen(header)) / 2, 2);

    auto github = "https://github.com/boyism80/fb";
    c.puts(github, c.width()-1 - strlen(github) - 3, 4);

    auto madeby = "made by cshyeon";
    c.puts(madeby, c.width()-1 - strlen(madeby) - 3, 5);

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    // Execute acceptor
    boost::asio::io_context io_service;
    auto& config = fb::config::get();
    acceptor = new fb::gateway::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

    io_service.run();

    // Clean up
    delete acceptor;
    c.release();

    return 0;
}