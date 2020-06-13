#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include "socket.h"
#include "fb_login.h"
#include "leak.h"
#include "console.h"
#include "resource.h"
#include "config.h"


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
    acceptor = new fb::login::acceptor(io_service, 2001);

    io_service.run();

    // Clean up
    delete acceptor;

    return 0;
}