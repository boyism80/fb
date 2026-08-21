#include <fb/game/listener.h>
#include <fb/game/server.h>
#include <fb/protocol/client_version.h>
#include <macro.h>

using namespace fb::game;
using namespace fb::model::enum_value;
using table = fb::model::table;

namespace game_resp = fb::protocol::game::response;

listener_impl::listener_impl(fb::game::server& server) :
    server(server)
{ }

namespace {

bool client_version_is(object& o, fb::protocol::CLIENT_VERSION ver)
{
    if (o.is(OBJECT_TYPE::CHARACTER) == false)
        return false;
    return static_cast<character&>(o).client_version == ver;
}

bool client_match(object& o, fb::protocol::CLIENT_VERSION ver, fb::protocol::CLIENT_UI_MODE ui_mode)
{
    if (o.is(OBJECT_TYPE::CHARACTER) == false)
        return false;

    auto& ch = static_cast<character&>(o);
    if (ch.client_version != ver)
        return false;
    if (ver != fb::protocol::CLIENT_VERSION::v651)
        return true;
    return ch.ui_mode == ui_mode;
}

template <fb::protocol::CLIENT_VERSION Ver>
void send_appearance_pivot(server&                       srv,
                           object&                       obj,
                           const character_appearance<>& app,
                           fb::protocol::CLIENT_UI_MODE  ui_mode)
{
    srv.send(obj,
             game_resp::show<Ver>(obj.oid(),
                                  obj.position(),
                                  obj.direction(),
                                  HEAD_MARKER::NONE,
                                  obj.name(),
                                  fb::game::character_appearance<Ver>(app),
                                  ui_mode),
             scope::PIVOT,
             {.with_me = client_match(obj, Ver, ui_mode), .condition = [ui_mode](object& o) {
                  return client_match(o, Ver, ui_mode);
              }});
}

void send_object_update_pivot(server& srv, object& me)
{
    using cv = fb::protocol::CLIENT_VERSION;
    srv.send(me,
             game_resp::update_v550(me),
             scope::PIVOT,
             {.with_me = client_version_is(me, cv::v550), .condition = [](object& o) {
                  return client_version_is(o, cv::v550);
              }});
    srv.send(me,
             game_resp::update_v565(me),
             scope::PIVOT,
             {.with_me = client_version_is(me, cv::v565), .condition = [](object& o) {
                  return client_version_is(o, cv::v565);
              }});
    srv.send(me,
             game_resp::update_v651(me),
             scope::PIVOT,
             {.with_me = client_version_is(me, cv::v651), .condition = [](object& o) {
                  return client_version_is(o, cv::v651);
              }});
}

} // namespace

void listener_impl::send_update_appearance(object& obj, const fb::model::appearance& appearance)
{
    auto app = character_appearance<>(appearance.gender,
                                      std::optional<STATE>(appearance.state),
                                      appearance.hair,
                                      appearance.hair_color,
                                      appearance.weapon,
                                      appearance.weapon_color,
                                      appearance.armor,
                                      appearance.armor_color,
                                      appearance.shield,
                                      appearance.shield_color,
                                      appearance.disguise);

    using cv = fb::protocol::CLIENT_VERSION;
    using um = fb::protocol::CLIENT_UI_MODE;
    send_appearance_pivot<cv::v550>(this->server, obj, app, um::OLD);
    send_appearance_pivot<cv::v565>(this->server, obj, app, um::OLD);
    send_appearance_pivot<cv::v651>(this->server, obj, app, um::OLD);
    send_appearance_pivot<cv::v651>(this->server, obj, app, um::NEW);
}

void listener_impl::send_update_appearance(object& obj, character& to, const fb::model::appearance& appearance)
{
    auto app = character_appearance<>(appearance.gender,
                                      std::optional<STATE>(appearance.state),
                                      appearance.hair,
                                      appearance.hair_color,
                                      appearance.weapon,
                                      appearance.weapon_color,
                                      appearance.armor,
                                      appearance.armor_color,
                                      appearance.shield,
                                      appearance.shield_color,
                                      appearance.disguise);

    fb::protocol::visit_client_version(to.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
        to.send(game_resp::show<Ver>(obj.oid(),
                                     obj.position(),
                                     obj.direction(),
                                     HEAD_MARKER::NONE,
                                     obj.name(),
                                     fb::game::character_appearance<Ver>(app),
                                     to.ui_mode));
    });
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

void listener_impl::send_non_character_external(object& me)
{
    switch (me.what())
    {
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
            send_object_update_pivot(this->server, me);
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
            send_object_update_pivot(this->server, me);
        }
    }
    break;

    default:
    {
        send_object_update_pivot(this->server, me);
    }
    break;
    }
}

void listener_impl::send_non_character_external(object& me, character& you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.model();
        if (model.appearance.has_value())
        {
            auto  appearance_table3 = table::appearance;
            auto& app               = appearance_table3[model.appearance.value()];
            this->send_update_appearance(npc, you, app);
        }
        else
        {
            fb::protocol::visit_client_version(you.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
                you.send(game_resp::update<Ver>(me));
            });
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
            this->send_update_appearance(mob, you, app);
        }
        else
        {
            fb::protocol::visit_client_version(you.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
                you.send(game_resp::update<Ver>(me));
            });
        }
    }
    break;

    default:
    {
        fb::protocol::visit_client_version(you.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
            you.send(game_resp::update<Ver>(me));
        });
    }
    break;
    }
}

void listener_impl::on_show(object& me)
{
    if (me.what() == OBJECT_TYPE::CHARACTER)
    {
        auto map = me.map();
        if (map == nullptr)
            return;

        for (auto& obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            if (me.hidden(*obj))
                continue;

            auto you = std::static_pointer_cast<character>(obj);
            fb::protocol::visit_client_version(you->client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
                you->send(game_resp::show<Ver>(static_cast<character&>(me), *you));
            });
        }
    }
    else
    {
        this->send_non_character_external(me);
    }
}

void listener_impl::on_show(object& me, object& you)
{
    if (me.hidden(you))
        return;

    if (you.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(you);
    if (me.what() == OBJECT_TYPE::CHARACTER)
    {
        fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
            ch.send(game_resp::show<Ver>(static_cast<character&>(me), ch));
        });
    }
    else
    {
        this->send_non_character_external(me, ch);
    }
}

void listener_impl::on_update_external(object& me)
{
    if (me.what() == OBJECT_TYPE::CHARACTER)
    {
        auto map = me.map();
        if (map == nullptr)
            return;

        for (auto& obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            if (me.hidden(*obj))
                continue;

            auto you = std::static_pointer_cast<character>(obj);
            fb::protocol::visit_client_version(you->client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
                you->send(game_resp::update_external<Ver>(static_cast<character&>(me), *you));
            });
        }
    }
    else
    {
        this->send_non_character_external(me);
    }
}

void listener_impl::on_update_external(object& me, object& you)
{
    if (me.hidden(you))
        return;

    if (you.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(you);
    if (me.what() == OBJECT_TYPE::CHARACTER)
    {
        fb::protocol::visit_client_version(ch.client_version, [&]<fb::protocol::CLIENT_VERSION Ver> {
            ch.send(game_resp::update_external<Ver>(static_cast<character&>(me), ch));
        });
    }
    else
    {
        this->send_non_character_external(me, ch);
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
    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(me);
    if (ENUM_IN(map.model().option, MAP_OPTION::BUILD_IN))
        ch.weather(WEATHER_TYPE::NORMAL);
    else
        ch.weather(this->server.weather.current());
}
