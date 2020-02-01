#include <stdio.h>
#include <iostream>
#include "socket.h"
#include "fb_game.h"
#include "db.h"
#include "leak.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "zlib/zlib.lib")
#if defined DEBUG | defined _DEBUG
#pragma comment(lib, "json/json_vc71_libmtd.lib")
#else
#pragma comment(lib, "json/json_vc71_libmt.lib")
#endif

fb::game::acceptor* acceptor = nullptr;

BOOL WINAPI handle_console(DWORD signal)
{
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

    // Load databases
    fb::game::db::loads();

    // Create acceptor instance
    acceptor = new fb::game::acceptor(10021);

    // Execute acceptor
    acceptor->execute(true);


    // Wait while key pressed
    _fgetchar();


    // Clean up
    WSACleanup();
    
    // Release
    delete acceptor;
    fb::game::db::release();

    return 0;
}