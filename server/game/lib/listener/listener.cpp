#include <fb/game/listener.h>
#include <fb/game/server.h>

using namespace fb::game;

listener_impl::listener_impl(fb::game::server& server) :
    server(server)
{ }

void listener_impl::send_update_preset(fb::game::object& obj, const fb::model::preset& preset)
{
    auto serializer = fb_resp::preset_serializer<true>{.oid         = obj.oid(),
                                                       .position    = obj.position(),
                                                       .direction   = obj.direction(),
                                                       .head_marker = HEAD_MARKER::NONE,
                                                       .name        = obj.name(),
                                                       .portrait    = character_portrait(preset.sex,
                                                                                      preset.state,
                                                                                      preset.hair,
                                                                                      preset.hair_color,
                                                                                      preset.armor,
                                                                                      preset.armor_color,
                                                                                      preset.weapon,
                                                                                      preset.weapon_color,
                                                                                      preset.shield,
                                                                                      preset.shield_color,
                                                                                      preset.disguise)};

    std::ignore = this->server.send(obj, fb_resp::update_external<true>(serializer), scope::PIVOT);
}

void listener_impl::on_create(object& me)
{
    return;
}

void listener_impl::on_destroy(object& me)
{
    return;
}

void listener_impl::on_chat(object& me, const std::string& message, CHAT_TYPE chat_type)
{
    if (me.is(OBJECT_TYPE::ITEM))
        return;

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

    std::ignore = this->server.send(me, fb_resp::chat(me, message, chat_type), scp);
}

void listener_impl::on_direction(object& me)
{
    std::ignore = this->server.send(me, fb_resp::direction(me), scope::PIVOT);
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

            auto you = std::static_pointer_cast<fb::game::character>(obj);
            if (detailed)
                you->send(fb_resp::update_external<true>(static_cast<character&>(me), *you));
            else
                you->send(fb_resp::update_external<false>(static_cast<character&>(me), *you));
        }
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.based<fb::model::npc>();
        if (model.preset.has_value())
        {
            auto& preset = this->server.model.preset[model.preset.value()];
            this->send_update_preset(npc, preset);
        }
        else
        {
            std::ignore = this->server.send(me, fb_resp::update(me), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.based<fb::model::mob>();
        if (model.preset.has_value())
        {
            auto& preset = this->server.model.preset[model.preset.value()];
            this->send_update_preset(mob, preset);
        }
        else
        {
            std::ignore = this->server.send(me, fb_resp::update(me), scope::PIVOT);
        }
    }
    break;

    default:
    {
        std::ignore = this->server.send(me, fb_resp::update(me), scope::PIVOT);
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
            you.send(fb_resp::update_external<true>(static_cast<character&>(me), you));
        else
            you.send(fb_resp::update_external<false>(static_cast<character&>(me), you));
    }
    break;

    case OBJECT_TYPE::NPC:
    {
        auto& npc   = static_cast<fb::game::npc&>(me);
        auto& model = npc.based<fb::model::npc>();
        if (model.preset.has_value())
        {
            auto& preset = this->server.model.preset[model.preset.value()];
            this->send_update_preset(npc, preset);
        }
        else
        {
            you.send(fb_resp::update(me));
        }
    }
    break;

    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.based<fb::model::mob>();
        if (model.preset.has_value())
        {
            auto& preset = this->server.model.preset[model.preset.value()];
            this->send_update_preset(mob, preset);
        }
        else
        {
            you.send(fb_resp::update(me));
        }
    }
    break;

    default:
    {
        you.send(fb_resp::update(me));
    }
    break;
    }
}

void listener_impl::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        std::ignore = this->server.send(me, fb_resp::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        std::ignore = this->server.send(me, fb_resp::die(static_cast<life&>(me)), scope::PIVOT, true);
        break;
    }
}

void listener_impl::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        you.send(fb_resp::hide(me));
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        you.send(fb_resp::die(static_cast<life&>(me)));
        break;
    }
}

void listener_impl::on_move(object& me, const fb::model::point16_t& before)
{
    std::ignore = this->server.send(me, fb_resp::move(me, before), scope::PIVOT, true);
}

void listener_impl::on_buff(object& me, buff& buff)
{
    me.send(fb::protocol::game::response::spell_buff(buff));
}

void listener_impl::on_unbuff(object& me, buff& buff)
{
    me.send(fb_resp::spell_unbuff(buff));
}

void listener_impl::on_sound(object& me, SOUND sound)
{
    std::ignore = this->server.send(me, fb_resp::sound(me, sound), scope::PIVOT);
}
void listener_impl::on_effect(object& me, uint8_t value)
{
    std::ignore = this->server.send(me, fb_resp::effect(me, value), scope::PIVOT);
}

void listener_impl::on_map_leave(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}

void listener_impl::on_map_enter(object& me, const fb::game::map& map)
{
    // Listener only handles packet response - no game logic
}