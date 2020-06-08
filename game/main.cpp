#include <stdio.h>
#include <iostream>
#include "socket.h"
#include "fb_game.h"
#include "db.h"
#include "leak.h"
#include "console.h"
#include "resource.h"
#include "lua.h"
#include "config.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "lua/lua5.3.5-static.lib")
#if defined DEBUG | defined _DEBUG
#pragma comment(lib, "json/json_vc71_libmtd.lib")
#pragma comment(lib, "zlib/zlibd.lib")
#else
#pragma comment(lib, "json/json_vc71_libmt.lib")
#pragma comment(lib, "zlib/zlib.lib")
#endif

fb::game::acceptor*  acceptor = nullptr;

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

    // Load databases
    fb::game::db::init();

    // Execute acceptor
    boost::asio::io_context io_service;
    acceptor = new fb::game::acceptor(io_service, 10021);
    io_service.run();


    // Wait while key pressed
    _fgetchar();


    // Release
    delete acceptor;
    fb::game::db::release();
    fb::game::lua::release();

    return 0;
}