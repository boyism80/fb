#include <stdio.h>
#include <iostream>
#include "socket.h"
#include "fb_game.h"
#include "leak.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "zlib/zlib.lib")
#if defined DEBUG | defined _DEBUG
#pragma comment(lib, "json/json_vc71_libmtd.lib")
#else
#pragma comment(lib, "json/json_vc71_libmt.lib")
#endif

using namespace fb::game;

BOOL WINAPI handle_console(DWORD signal)
{
    acceptor*       acceptor = fb::game::acceptor::instance();

    switch(signal)
    {
    case CTRL_C_EVENT:
        acceptor->exit();
        puts("Please wait to exit acceptor.");
        break;
    }

    return true;
}

int main(int argc, const char** argv)
{
    ::SetConsoleCtrlHandler(handle_console, true);

    // Initialization
    WSADATA                 wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 0;


    // Execute acceptor
    acceptor*       acceptor = fb::game::acceptor::instance();
    acceptor->execute(true);


    _fgetchar();

    // Clean up
    WSACleanup();
    fb::game::acceptor::release();

    return 0;
}