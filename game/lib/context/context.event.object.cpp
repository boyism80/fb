#include <fb/game/context.h>

using namespace fb::game;

void context::on_create(object& me)
{
    return;
}

void context::on_destroy(object& me)
{
    return;
}

void context::on_chat(object& me, const std::string& message, CHAT_TYPE chat_type)
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
    this->send(me, fb_resp::chat(me, message, chat_type), scp);
}

void context::on_direction(object& me)
{
    auto lua = fb::lua::new_context();
#if defined DEBUG | defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif
    lua->func("on_direction");
    lua->pushobject(me);
    lua->call(1);

    this->send(me, fb_resp::direction(me), scope::PIVOT);
}

void context::on_update_external(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto map = me.map();
        if (map == nullptr)
            return;

        for (auto obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            auto you = static_cast<character*>(obj);
            you->send(fb_resp::update_external(static_cast<character&>(me), *you, light));
        }
    }
    else
    {
        this->send(me, fb_resp::update(me), scope::PIVOT);
    }
}

void context::on_update_external(object& me, object& you, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
        you.send(fb_resp::update_external(static_cast<character&>(me), you, light));
    else
        you.send(fb_resp::update(me));
}

void context::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->send(me, fb_resp::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::die(static_cast<life&>(me)), scope::PIVOT, true);
        break;
    }
}

void context::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        me.send(fb_resp::hide(you));
        break;

    case DESTROY_TYPE::DEAD:
        if (you.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        me.send(fb_resp::die(static_cast<life&>(you)));
        break;
    }
}

void context::on_move(object& me, const fb::model::point16_t& before)
{
    auto lua = fb::lua::new_context();
#if defined DEBUG | defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif
    lua->func("on_move");
    lua->pushobject(me);
    lua->call(1);

    this->send(me, fb_resp::move(me, before), scope::PIVOT, true);
}

void context::on_buff(object& me, buff& buff)
{
    if (buff.model.buff == "")
        return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return;

    lua->func(buff.model.buff);
    lua->pushobject(me);
    lua->pushobject(buff.model);
    lua->call(2);

    me.send(fb::protocol::game::response::spell_buff(buff));
}

void context::on_unbuff(object& me, buff& buff)
{
    if (buff.model.unbuff == "")
        return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return;

    lua->func(buff.model.unbuff);
    lua->pushobject(me);
    lua->pushobject(buff.model);
    lua->call(2);
    me.send(fb_resp::spell_unbuff(buff));

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch = static_cast<character&>(me);
        ch.message(std::format("{} 해제", buff.model.name));
    }
}

void context::on_sound(object& me, SOUND sound)
{
    this->send(me, fb_resp::sound(me, sound), scope::PIVOT);
}
void context::on_effect(object& me, uint8_t value)
{
    this->send(me, fb_resp::effect(me, value), scope::PIVOT);
}