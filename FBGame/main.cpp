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

using namespace fb::game;

fb::game::acceptor* acceptor_instance = nullptr;

BOOL WINAPI handle_console(DWORD signal)
{
    switch(signal)
    {
    case CTRL_C_EVENT:
        acceptor_instance->exit();
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
    db::loads();

    // Create acceptor instance
    acceptor_instance = new fb::game::acceptor(10021);

    // Execute acceptor
    acceptor_instance->execute(true);


    // Wait while key pressed
    _fgetchar();


    // Clean up
    WSACleanup();
    
    // Release
    delete acceptor_instance;
    db::release();

    return 0;
}