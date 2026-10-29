#include <fb/amqp_route.h>
#include <fb/encoding.h>
#include <fb/game/handler.h>
#include <fb/game/server.h>
#include <fb/log_collector.h>

#include <json/json.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <tuple>

using namespace fb::game;
using namespace fb::model::enum_value;
using namespace std::chrono_literals;

namespace game_reqs     = fb::protocol::game::request;
namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;

bool fb::game::server::assert_tps(const fb::socket<fb::game::character>& socket) const
{
    // Called from the IO thread while a logic thread may release the session.
    auto ch = socket.data_ptr();
    if (ch == nullptr)
        return true;

    return ch->role() == ROLE::USER;
}

async::task<bool> fb::game::server::on_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<void> fb::game::server::unload(character& ch)
{
    this->matches.leave(ch);

    auto  weak     = ch.weak_from_this_as<character>();
    auto& group_id = ch.group.id();
    if (group_id.has_value())
    {
        auto gid = group_id.value();
        try
        {
            this->groups.write(gid, [weak](auto& group) {
                group->detach(weak);
            });
        }
        catch (const std::runtime_error&)
        { }
        ch.group.reset();
        this->groups.update_portraits(gid);
    }

    auto& clan_id = ch.clan.id();
    if (clan_id.has_value())
    {
        try
        {
            this->clans.write(clan_id.value(), [weak](auto& clan) {
                clan->detach(weak);
            });
        }
        catch (const std::runtime_error&)
        { }
        ch.clan.reset();
    }

    // remove() erases by id and name; a failed duplicate insert must not evict the character already online.
    auto self = ch.shared_from_this_as<character>();
    if (this->characters.find(ch.id) == self)
        this->characters.remove(self);
    co_await ch.destroy();
}

async::task<bool> fb::game::server::on_disconnected(fb::socket<character>& socket)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;

    // A character still loading is cleaned up and logged out by the login handler.
    if (ch->abort_login())
        co_return false;

    auto weak = ch->weak_from_this_as<character>();

    if (ch->trade != nullptr)
        std::ignore = ch->trade->cancel(*ch);

    // A script waiting on a dialog keeps the character alive, so it must not outlive the session.
    ch->dialog.cancel("disconnected");

    fb::logger::info("{} has disconnected.", ch->name());

    auto thread      = ch->thread();
    auto world       = ch->world();
    auto name        = ch->name();
    auto transferred = ch->transferred();
    auto saved       = false;
    try
    {
        if (this->running())
        {
            co_await this->save(*ch);
        }
        saved = true;
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
    catch (std::exception&)
    {
        // The character expired before the switch; fall back to the thread it was on.
    }

    if (!switched)
        co_await thread->switching();

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        co_await ptr->matchmaker.dequeue(matchmaker::initiator::SERVER);

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

        co_await this->unload(*ptr);
        socket.data(nullptr);
    }

    if (saved && transferred == false)
    {
        try
        {
            auto resp = co_await this->http.post("internal",
                                                 "/in-game/logout",
                                                 internal_reqs::Logout{world, name, fb::config<uint8_t>("id")});
            if (resp.success == false)
                fb::logger::fatal("Character {} logout failed: {}", name, resp.reason.value_or(""));
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
    }

    co_return true;
}

async::task<void> fb::game::server::on_exit()
{
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

    co_await this->save();
    co_await this->maps.cleanup();
    co_return;
}

void fb::game::server::on_init_amqp(fb::amqp::socket& amqp)
{
    auto scope = fb::amqp_scope();
    auto id    = fb::config<uint32_t>("id");
    this->handler.amqp.declare_queue("amq.direct", "fb.global");
    for (auto& t : fb::k_amqp_topics)
    {
        if (std::string_view(t.name) == "castle" && scope == "cross")
            continue;
        this->handler.amqp.declare_queue("amq.direct", fb::amqp_key(t.name, scope, id));
    }
}
