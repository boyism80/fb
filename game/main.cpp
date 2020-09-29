#include "resource.h"
#include <stdio.h>
#include <iostream>
#include "module/socket/socket.h"
#include "module/config/config.h"
#include "module/console/console.h"
#include "module/leak.h"
#include "model/acceptor/acceptor.game.h"
#include "model/master/master.h"
#include "model/lua/lua.h"

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(157);

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    // Execute acceptor
    auto& config = fb::config();
    boost::asio::io_context io_service;
    auto acceptor = new fb::game::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

    fb::game::master::get().loads(acceptor);
    io_service.run();

    // Release
    delete acceptor;
    fb::game::lua::release();
    fb::game::master::release();

    return 0;
}