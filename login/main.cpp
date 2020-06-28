#include <stdio.h>
#include <iostream>
#include <mysql+++/mysql+++.h>
#include "module/socket/socket.h"
#include "model/acceptor/acceptor.login.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "resource.h"
#include "module/config/config.h"

#pragma comment(lib, "libmysql.lib")

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
    connection my{ "localhost", "root", "tmdgus12", "fb" };
    if (!my) {
        std::cout << "Connection failed" << std::endl;
        return 0;
    }

    auto row_count = my.query("select count(*) from item").get_value<int>();

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