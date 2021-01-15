#include "resource.h"
#include "module/db/db.h"
#include "model/acceptor/acceptor.login.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "module/config/config.h"
#include "protocol/internal.h"

fb::login::acceptor*       acceptor;

int main(int argc, const char** argv)
{
    //_CrtSetBreakAlloc(165);

#ifdef _WIN32
    ::SetConsoleIcon(IDI_BARAM);
    ::SetConsoleTitle(CONSOLE_TITLE);
#endif

    auto& c = fb::console::get();
    auto height = 8;
    c.box(0, 0, c.width()-1, height);

    auto header = "The Kingdom of the wind [LOGIN]";
    c.move((c.width()-1 - strlen(header)) / 2, 2).puts(header);

    auto github = "https://github.com/boyism80/fb";
    c.move(c.width()-1 - strlen(github) - 3, 4).puts(github);

    auto madeby = "made by cshyeon";
    c.move(c.width()-1 - strlen(madeby) - 3, 5).puts(madeby);

    c.current_line(height + 1);

    // Execute acceptor
    boost::asio::io_context io_context;
    fb::db::bind(io_context);
    auto& config = fb::config::get();

    const auto connection = INTERNAL_CONNECTION
    {
        config["internal"]["ip"].asString(),
        (uint16_t)config["internal"]["port"].asInt(),
        [&] (fb::base::socket<>& socket, bool success)
        {
            if(success)
                socket.send(fb::protocol::internal::request::subscribe(config["id"].asString()));
        },
        [&] ()
        {
            // on disconnected
        }
    };
    acceptor = new fb::login::acceptor
    (
        io_context, 
        config["port"].asInt(), 
        config["delay"].asInt(), 
        connection
    );

    io_context.run();

    // Clean up
    delete acceptor;
    fb::console::release();
    fb::db::close();
    return 0;
}