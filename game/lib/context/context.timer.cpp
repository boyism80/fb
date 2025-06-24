#include <fb/game/context.h>

using namespace fb::game;

async::task<void> context::handle_heart_beat()
{
    auto root    = Json::Value{};
    root["Name"] = this->name();
    root["IP"]   = fb::config<std::string>("ip");
    root["Port"] = fb::config<uint16_t>("port");
    auto writer  = Json::FastWriter{};
    auto output  = writer.write(root);

    this->_redis.command<void>(std::format("SET heart-beat:Game:{} {}", this->id(), output));
    this->_redis.command<void>(std::format("EXPIRE heart-beat:Game:{} 5", this->id()));
    co_return;
}

async::task<void> fb::game::context::handle_announce()
{
    auto i = random<uint16_t>(0, this->model.announce.size());
    co_await this->broadcast(this->model.announce[i].message, MESSAGE_TYPE::WORLD, BROADCAST_TYPE::WORLD);
}

async::task<void> context::handle_time()
{
    auto updated = fb::model::datetime();
    if (this->_time.hours() != updated.hours())
    {
        this->characters.foreach ([hours = updated.hours()](auto& ch) {
            ch->update_time(hours);
        });
    }

    this->_time = updated;
    co_return;
}