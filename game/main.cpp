#include "resource.h"
#include <stdio.h>
#include <iostream>
#include <module/socket/socket.h>
#include <module/config/config.h>
#include <module/console/console.h>
#include <module/leak.h>
#include <model/acceptor/acceptor.game.h>
#include <model/master/master.h>
#include <model/lua/lua.h>

BOOL WINAPI handle_console(DWORD signal)
{
    switch(signal)
    {
    case CTRL_C_EVENT:
        //acceptor->exit();
        puts("Please wait to exit acceptor.");
        break;
    }

    return true;
}

int main(int argc, const char** argv)
{
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
    ::SetConsoleCtrlHandler(handle_console, true);

    //_CrtSetBreakAlloc(157);

    // Execute acceptor
    boost::asio::io_context io_service;
    auto acceptor = new fb::game::acceptor(io_service, fb::config()["port"].asInt());

    fb::game::master::get().loads(acceptor);
    io_service.run();

    // Release
    delete acceptor;
    fb::game::lua::release();
    fb::game::master::release();

    return 0;
}