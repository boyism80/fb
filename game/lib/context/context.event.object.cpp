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

    auto sstream = std::stringstream();
    switch (chat_type)
    {
    case CHAT_TYPE::SHOUT:
        sstream << me.name() << "! " << message;
        break;

    default:
        sstream << me.name() << ": " << message;
        break;
    }

    auto scp = scope::PIVOT;
    switch (chat_type)
    {
    case CHAT_TYPE::SHOUT:
        scp = scope::MAP;
        break;

    case CHAT_TYPE::BLUE:
    case CHAT_TYPE::LIGHT_BLUE:
        scp = scope::WORLD;
        break;

    default:
        scp = scope::PIVOT;
        break;
    }
    this->send(me, fb_resp::chat(me, sstream.str(), chat_type), scp);
}

void context::on_direction(object& me)
{
    this->send(me, fb_resp::direction(me), scope::PIVOT, true);
}

void context::on_show(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        this->send(
            me,
            [&me, light](const auto& to) {
                return std::unique_ptr<fb::protocol::header>(
                    new fb_resp::update_external(static_cast<character&>(me), to, light));
            },
            scope::PIVOT);
    }
    else
    {
        this->send(me, fb_resp::update(me), scope::PIVOT);
    }
}

void context::on_show(object& me, object& you, bool light)
{
    if (you.is(OBJECT_TYPE::CHARACTER))
        this->send(me, fb_resp::update_external(static_cast<character&>(you), me, light), scope::SELF);
    else
        this->send(me, fb_resp::update(you), scope::SELF);
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
        this->send(me, fb_resp::hide(you), scope::SELF);
        break;

    case DESTROY_TYPE::DEAD:
        if (you.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::die(static_cast<life&>(you)), scope::SELF);
        break;
    }
}

void context::on_move(object& me, const point16_t& before)
{
    this->send(me, fb_resp::move(me, before), scope::PIVOT, true);
}

void context::on_unbuff(object& me, buff& buff)
{
    if (buff.model.uncast.empty())
        return;

    auto thread = lua::get();
    if (thread == nullptr)
        return;
    thread->from(buff.model.uncast.c_str()).func("on_uncast").pushobject(me).pushobject(buff.model).resume(2);
    this->send(me, fb_resp::spell_unbuff(buff), scope::SELF);
}

void context::on_hold(fb::game::object& me)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        this->send(me, fb_resp::position(static_cast<character&>(me)), scope::SELF);
    }
}