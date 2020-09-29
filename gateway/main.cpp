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

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    // Execute acceptor
    boost::asio::io_context io_service;
    auto& config = fb::config();
    acceptor = new fb::gateway::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

    io_service.run();

    // Clean up
    delete acceptor;

    return 0;
}