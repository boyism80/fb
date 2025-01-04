#include <fb/game/context.h>

using namespace fb::game;

async::task<void> context::handle_heart_beat()
{
    std::ignore = co_await this->post<internal_reqs::Ping, internal_resp::Pong>(
        "internal",
        "/in-game/ping",
        internal_reqs::Ping{this->id(),
                            this->name(),
                            this->service(),
                            fb::config<std::string>("ip"),
                            fb::config<uint16_t>("port")});
}

async::task<void> context::handle_time()
{
    auto updated = fb::model::datetime();
    if (this->_time.hours() != updated.hours())
    {
        this->foreach_ch([hours = updated.hours()](auto& ch) -> async::task<void> {
            ch.update_time(hours);
            co_return;
        });
    }

    this->_time = updated;
    co_return;
}