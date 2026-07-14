#include <fb/game/listener.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <tuple>

using namespace fb::game;
using table = fb::model::table;

namespace game_resp = fb::protocol::game::response;

listener_impl::listener_impl(fb::game::server& server) :
    server(server)
{ }

async::task<void> listener_impl::send_update_appearance(object& obj, const fb::model::appearance& appearance)
{
    auto serializer = game_resp::appearance_serializer<true>{
        .oid         = obj.oid(),
        .position    = obj.position(),
        .direction   = obj.direction(),
        .head_marker = HEAD_MARKER::NONE,
        .name        = obj.name(),
        .appearance  = character_appearance(appearance.gender,
                                           std::optional<STATE>(appearance.state),
                                           appearance.hair,
                                           appearance.hair_color,
                                           appearance.weapon,
                                           appearance.weapon_color,
                                           appearance.armor,
                                           appearance.armor_color,
                                           appearance.shield,
                                           appearance.shield_color,
                                           appearance.disguise)};

    co_await this->server.send(obj, game_resp::update_external<true>(serializer), scope::PIVOT);
}

void listener_impl::on_create(object& me)
{
    return;
}

void listener_impl::on_destroy(object& me)
{
    return;
}

async::task<void> listener_impl::on_chat(object& me, std::string_view message, CHAT_TYPE chat_type)
{
    if (me.is(OBJECT_TYPE::ITEM))
        co_return;

    auto scp = scope::PIVOT;
    switch (chat_type)
    {
    case CHAT_TYPE::SHOUT:
        scp = scope::MAP;
        break;

    default:
        scp = scope::PIVOT;
        break;
    }

    co_await this->server.send(me, game_resp::chat(me, message, chat_type), scp);
}

async::task<void> listener_impl::on_direction(object& me)
{
    co_await this->server.send(me, game_resp::direction(me), scope::PIVOT);
}

async::task<void> listener_impl::on_update_external(object& me, bool detailed)
{
    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        auto map = me.map();
        if (map == nullptr)
            co_return;

        for (auto& obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            if (me.hidden(*obj))
                continue;

            auto you = std::static_pointer_cast<character>(obj);
            try
            {
                if (detailed)
                    std::ignore =
                        co_await you->send(game_resp::update_external<true>(static_cast<character&>(me), *you));
                else
                    std::ignore =
                        co_await you->send(game_resp::update_external<false>(static_cast<character&>(me), *you));
            }
            catch (const std::exception& e)
            {
                fb::logger::fatal("on_update_external recipient failed (oid={}): {}", you->oid(), e.what());
            }
        }
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.based<fb::model::npc>();
        if (model.appearance.has_value())
        {
            auto& app = table::appearance[model.appearance.value()];
            co_await this->send_update_appearance(npc, app);
        }
        else
        {
            co_await this->server.send(me, game_resp::update(me), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.based<fb::model::mob>();
        if (model.appearance.has_value())
        {
            auto& app = table::appearance[model.appearance.value()];
            co_await this->send_update_appearance(mob, app);
        }
        else
        {
            co_await this->server.send(me, game_resp::update(me), scope::PIVOT);
        }
    }
    break;

    default:
    {
        co_await this->server.send(me, game_resp::update(me), scope::PIVOT);
    }
    break;
    }
}

async::task<void> listener_impl::on_update_external(object& me, object& you, bool detailed)
{
    if (me.hidden(you))
        co_return;

    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        if (detailed)
            std::ignore = co_await you.send(game_resp::update_external<true>(static_cast<character&>(me), you));
        else
            std::ignore = co_await you.send(game_resp::update_external<false>(static_cast<character&>(me), you));
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.based<fb::model::npc>();
        if (model.appearance.has_value())
        {
            auto& app = table::appearance[model.appearance.value()];
            co_await this->send_update_appearance(npc, app);
        }
        else
        {
            std::ignore = co_await you.send(game_resp::update(me));
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.based<fb::model::mob>();
        if (model.appearance.has_value())
        {
            auto& app = table::appearance[model.appearance.value()];
            co_await this->send_update_appearance(mob, app);
        }
        else
        {
            std::ignore = co_await you.send(game_resp::update(me));
        }
    }
    break;

    default:
    {
        std::ignore = co_await you.send(game_resp::update(me));
    }
    break;
    }
}

async::task<void> listener_impl::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        co_await this->server.send(me, game_resp::hide(me), scope::PIVOT, {.with_me = false});
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        co_await this->server.send(me, game_resp::die(static_cast<life&>(me)), scope::PIVOT, {.with_me = false});
        break;
    }
}

async::task<void> listener_impl::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        std::ignore = co_await you.send(game_resp::hide(me));
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        std::ignore = co_await you.send(game_resp::die(static_cast<life&>(me)));
        break;
    }
}

async::task<void> listener_impl::on_move(object& me, const fb::model::point16_t& before)
{
    co_await this->server.send(me, game_resp::move(me, before), scope::PIVOT, {.with_me = false});
}

async::task<void> listener_impl::on_buff(object& me, buff& buff)
{
    std::ignore = co_await me.send(fb::protocol::game::response::spell_buff(buff));
}

async::task<void> listener_impl::on_unbuff(object& me, buff& buff)
{
    std::ignore = co_await me.send(game_resp::spell_unbuff(buff));
}

async::task<void> listener_impl::on_sound(object& me, SOUND sound)
{
    co_await this->server.send(me, game_resp::sound(me, sound), scope::PIVOT);
}

async::task<void> listener_impl::on_effect(object& me, uint8_t value)
{
    co_await this->server.send(me, game_resp::effect(me, value), scope::PIVOT);
}

void listener_impl::on_map_leave(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_map_enter(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}
