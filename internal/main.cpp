#include "model/acceptor/acceptor.internal.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "module/config/config.h"

fb::internal::acceptor*       acceptor;

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(165);

    auto& c = fb::console::get();
    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [INTERNAL]";
    c.move((c.width()-1 - strlen(header)) / 2, 2).puts(header);

    auto github = "https://github.com/boyism80/fb";
    c.move(c.width()-1 - strlen(github) - 3, 4).puts(github);

    auto madeby = "made by cshyeon";
    c.move(c.width()-1 - strlen(madeby) - 3, 5).puts(madeby);

    c.current_line(height + 1);

    // Execute acceptor
    boost::asio::io_context io_service;
    auto& config = fb::config::get();
    acceptor = new fb::internal::acceptor(io_service, config["port"].asInt(), config["delay"].asInt());

    io_service.run();

    // Clean up
    delete acceptor;
    c.release();

    return 0;
}