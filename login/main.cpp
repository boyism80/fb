#include <stdio.h>
#include <iostream>
#include "resource.h"
#include <mysql+++/mysql+++.h>
#include <module/socket/socket.h>
#include <model/acceptor/acceptor.login.h>
#include <module/leak.h>
#include <module/console/console.h>
#include <module/config/config.h>

using namespace daotk::mysql;

fb::login::acceptor*       acceptor;

BOOL WINAPI handle_console(DWORD signal)
{
    switch(signal)
    {
    case CTRL_C_EVENT:
        //acceptor_login->exit();
        puts("Please wait to exit acceptor.");
        break;
    }

    return true;
}

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(165);

    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
    ::SetConsoleCtrlHandler(handle_console, true);

    // Execute acceptor
    boost::asio::io_context io_service;
    acceptor = new fb::login::acceptor(io_service, fb::config()["port"].asInt());

    io_service.run();

    // Clean up
    delete acceptor;

    return 0;
}