#include <fb/game/server.h>
#include <fb/game/handler.h>
#include <fb/log_collector.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <format>
#include <tuple>

using namespace fb::game;
using namespace fb::model::enum_value;
using namespace std::chrono_literals;

namespace game_reqs     = fb::protocol::game::request;
namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;

bool fb::game::server::decrypt_policy(uint8_t opcode) const
{
    switch (opcode)
    {
    // The opcode is version independent, so any instantiation resolves it.
    case game_reqs::login<fb::protocol::CLIENT_VERSION::v550>::opcode:
        return false;

    default:
        return true;
    }
}

bool fb::game::server::assert_tps(const fb::socket<fb::game::character>& socket) const
{
    auto ch = socket.data();
    if (ch == nullptr)
        return true;

    return ch->role() == ROLE::USER;
}

async::task<bool> fb::game::server::on_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<bool> fb::game::server::on_disconnected(fb::socket<character>& socket)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;

    auto weak = ch->weak_from_this_as<character>();

    if (ch->trade.trading())
        std::ignore = ch->trade.cancel();

    fb::logger::info("{} has disconnected.", ch->name());

    auto thread = ch->thread();
    try
    {
        if (this->running())
        {
            co_await this->save(*ch);
        }
        auto world  = fb::config<uint32_t>("world");
        std::ignore = co_await this->http.post("internal", "/in-game/logout", internal_reqs::Logout{world, ch->name()});
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Check if character is still valid before thread switching
    auto switched = false;
    try
    {
        co_await this->threads.switching(weak);
        switched = true;
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Thread switching failed during disconnect: {}", e.what());
    }

    if (!switched)
        co_await thread->switching();

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        co_await ptr->matchmaker.unregister_queue(true);

        // Log logout event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
        log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
        log_data["level"]          = ptr->level();
        auto map                   = ptr->map();
        if (map != nullptr)
        {
            log_data["map"]        = map->model().id;
            log_data["position_x"] = ptr->position().x;
            log_data["position_y"] = ptr->position().y;
        }
        this->log.write("logout", log_data);

        auto& group_id = ptr->group_id();
        if (group_id.has_value())
        {
            auto gid = group_id.value();
            this->groups.write(gid, [weak](auto& group) {
                group->detach(weak);
            });
            ptr->group_reset();
            this->groups.update_portraits(gid);
        }

        auto& clan_id = ptr->clan_id();
        if (clan_id.has_value())
        {
            this->clans.write(clan_id.value(), [weak](auto& clan) {
                clan->detach(weak);
            });
            ptr->clan_reset();
        }

        this->characters.remove(ptr);
        co_await ch->destroy();
        socket.data(nullptr);
    }
    else
    {
        fb::logger::debug("Character expired during cleanup, skipping group/clan operations");
    }

    co_return true;
}

async::task<void> fb::game::server::on_exit()
{
    co_await this->save();

    while (true)
    {
        auto pending = size_t{0};
        for (uint8_t i = 0; i < this->threads.count(); ++i)
        {
            auto thread = this->threads.at(i);
            if (thread != nullptr)
                pending += thread->queue_size();
        }

        if (pending == 0)
            break;

        co_await this->sleep(10ms);
    }

    co_await this->maps.cleanup();
    co_return;
}

uint32_t fb::game::server::thread_id(const fb::socket<character>& socket) const
{
    auto character = socket.data();
    if (character == nullptr)
        throw std::runtime_error("character is empty");

    auto map = character->map();
    if (map == nullptr)
        return 0;

    return map->model().id;
}

void fb::game::server::on_init_amqp(fb::amqp::socket& amqp)
{
    auto world = config<uint32_t>("world");
    this->handler.amqp.declare_queue("amq.direct", "fb.global");                        // Shutdown: all servers
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.system", world)); // System mail, broadcast save
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.game.{}", world, fb::config<uint32_t>("id")));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.global", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.group", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.clan", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.mail", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.storage", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.ban", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.matchmaking", world));
}
