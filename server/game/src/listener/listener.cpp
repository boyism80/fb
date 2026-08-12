#include <fb/game/listener.h>
#include <fb/game/server.h>
#include <fb/protocol/client_version.h>

using namespace fb::game;
using table = fb::model::table;

namespace game_resp = fb::protocol::game::response;

listener_impl::listener_impl(fb::game::server& server) :
    server(server)
{ }

void listener_impl::send_update_appearance(object& obj, const fb::model::appearance& appearance)
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

    this->server.send(obj, game_resp::update_external<true>(serializer), scope::PIVOT);
}

void listener_impl::on_create(object& me)
{
    return;
}

void listener_impl::on_destroy(object& me)
{
    return;
}

void listener_impl::on_chat(object& me, std::string_view message, CHAT_TYPE chat_type)
{
    if (me.is(OBJECT_TYPE::ITEM))
        return;

    if (chat_type == CHAT_TYPE::SHOUT)
    {
        this->server.send(me, game_resp::chat(me, message, chat_type), scope::MAP, {.condition = [&me](object& to) {
                              if (to.is(OBJECT_TYPE::CHARACTER) == false)
                                  return false;

                              auto& ch = static_cast<character&>(to);
                              if (ch.sight(me) && me.hidden(ch) == false)
                                  return true;

                              return ch.option(OPTION::ROAR);
                          }});
    }
    else
    {
        this->server.send(me, game_resp::chat(me, message, chat_type), scope::PIVOT);
    }
}

void listener_impl::on_direction(object& me)
{
    this->server.send(me, game_resp::direction(me), scope::PIVOT);
}

void listener_impl::on_update_external(object& me, bool detailed)
{
    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        auto map = me.map();
        if (map == nullptr)
            return;

        for (auto& obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            if (me.hidden(*obj))
                continue;

            auto you = std::static_pointer_cast<character>(obj);
            if (detailed)
                you->send(game_resp::update_external<true>(static_cast<character&>(me), *you));
            else
                you->send(game_resp::update_external<false>(static_cast<character&>(me), *you));
        }
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.model();
        if (model.appearance.has_value())
        {
            auto  appearance_table = table::appearance;
            auto& app              = appearance_table[model.appearance.value()];
            this->send_update_appearance(npc, app);
        }
        else
        {
            this->server.send(me, game_resp::update(me), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.model();
        if (model.appearance.has_value())
        {
            auto  appearance_table2 = table::appearance;
            auto& app               = appearance_table2[model.appearance.value()];
            this->send_update_appearance(mob, app);
        }
        else
        {
            this->server.send(me, game_resp::update(me), scope::PIVOT);
        }
    }
    break;

    default:
    {
        this->server.send(me, game_resp::update(me), scope::PIVOT);
    }
    break;
    }
}

void listener_impl::on_update_external(object& me, object& you, bool detailed)
{
    if (me.hidden(you))
        return;

    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        if (detailed)
            you.send(game_resp::update_external<true>(static_cast<character&>(me), you));
        else
            you.send(game_resp::update_external<false>(static_cast<character&>(me), you));
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.model();
        if (model.appearance.has_value())
        {
            auto  appearance_table3 = table::appearance;
            auto& app               = appearance_table3[model.appearance.value()];
            this->send_update_appearance(npc, app);
        }
        else
        {
            you.send(game_resp::update(me));
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.model();
        if (model.appearance.has_value())
        {
            auto  appearance_table4 = table::appearance;
            auto& app               = appearance_table4[model.appearance.value()];
            this->send_update_appearance(mob, app);
        }
        else
        {
            you.send(game_resp::update(me));
        }
    }
    break;

    default:
    {
        you.send(game_resp::update(me));
    }
    break;
    }
}

void listener_impl::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->server.send(me, game_resp::hide(me), scope::PIVOT, {.with_me = false});
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->server.send(me, game_resp::die(static_cast<life&>(me)), scope::PIVOT, {.with_me = false});
        break;
    }
}

void listener_impl::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        you.send(game_resp::hide(me));
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        you.send(game_resp::die(static_cast<life&>(me)));
        break;
    }
}

void listener_impl::on_move(object& me, const fb::model::point16_t& before)
{
    this->server.send(me, game_resp::move(me, before), scope::PIVOT, {.with_me = false});
}

void listener_impl::on_buff(object& me, buff& buff)
{
    me.send(fb::protocol::game::response::spell_buff(buff));
}

void listener_impl::on_unbuff(object& me, buff& buff)
{
    me.send(game_resp::spell_unbuff(buff));
}

void listener_impl::on_sound(object& me, SOUND sound, uint8_t volume)
{
    this->server.send(me, game_resp::sound(me, sound, volume), scope::PIVOT);
}
void listener_impl::on_effect(object& me, uint8_t value)
{
    using cv     = fb::protocol::CLIENT_VERSION;
    auto matches = [](object& o, cv ver) -> bool {
        if (o.is(OBJECT_TYPE::CHARACTER) == false)
            return false;
        return static_cast<character&>(o).client_version == ver;
    };

    this->server.send(me,
                      game_resp::effect_v550(me, value),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v550), .condition = [matches](object& o) {
                           return matches(o, cv::v550);
                       }});
    this->server.send(me,
                      game_resp::effect_v565(me, value),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v565), .condition = [matches](object& o) {
                           return matches(o, cv::v565);
                       }});
    this->server.send(me,
                      game_resp::effect_v651(me, value),
                      scope::PIVOT,
                      {.with_me = matches(me, cv::v651), .condition = [matches](object& o) {
                           return matches(o, cv::v651);
                       }});
}

void listener_impl::on_map_leave(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_map_enter(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}