#include <fb/game/listener.h>
#include <fb/game/context.h>

using namespace fb::game;

listener_impl::listener_impl(fb::game::context& ctx) :
    context(ctx)
{ }

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

    this->context.send(me, fb_resp::chat(me, message, chat_type), scp);
}

void listener_impl::on_direction(object& me)
{
    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
#endif
        lua->func("on_direction");
        lua->pushobject(me);
        std::ignore = lua->call(1);
    }

    this->context.send(me, fb_resp::direction(me), scope::PIVOT);
}

void listener_impl::on_update_external(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto map = me.map();
        if (map == nullptr)
            return;

        for (auto obj : map->nears(me.position(), OBJECT_TYPE::CHARACTER))
        {
            if (me.hidden(*obj))
                continue;

            auto you = std::static_pointer_cast<fb::game::character>(obj);
            you->send(fb_resp::update_external(static_cast<character&>(me), *you, light));
        }
    }
    else
    {
        this->context.send(me, fb_resp::update(me), scope::PIVOT);
    }
}

void listener_impl::on_update_external(object& me, object& you, bool light)
{
    if (me.hidden(you))
        return;

    if (me.is(OBJECT_TYPE::CHARACTER))
        you.send(fb_resp::update_external(static_cast<character&>(me), you, light));
    else
        you.send(fb_resp::update(me));
}

void listener_impl::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->context.send(me, fb_resp::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->context.send(me, fb_resp::die(static_cast<life&>(me)), scope::PIVOT, true);
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
    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
#endif
        lua->func("on_move");
        lua->pushobject(me);
        std::ignore = lua->call(1);
    }

    this->context.send(me, fb_resp::move(me, before), scope::PIVOT, true);
}

void listener_impl::on_buff(object& me, buff& buff)
{
    if (buff.model.buff.empty())
        return;

    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
        lua->func(buff.model.buff);
        lua->pushobject(me);
        lua->pushobject(buff.model);
        std::ignore = lua->call(2);
    }

    me.send(fb::protocol::game::response::spell_buff(buff));
}

void listener_impl::on_unbuff(object& me, buff& buff)
{
    if (buff.model.unbuff.empty())
        return;

    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
        lua->func(buff.model.unbuff);
        lua->pushobject(me);
        lua->pushobject(buff.model);
        std::ignore = lua->call(2);
    }

    me.send(fb_resp::spell_unbuff(buff));

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch = static_cast<character&>(me);
        ch.message(std::format("{} 해제", buff.model.name));
    }
}

void listener_impl::on_sound(object& me, SOUND sound)
{
    this->context.send(me, fb_resp::sound(me, sound), scope::PIVOT);
}
void listener_impl::on_effect(object& me, uint8_t value)
{
    this->context.send(me, fb_resp::effect(me, value), scope::PIVOT);
}

void listener_impl::on_map_changed(object& me, const fb::game::map* before, const fb::game::map* after)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch = static_cast<character&>(me);
        if (before != nullptr)
        {
            auto thread = before->thread();
            if (thread != nullptr)
            {
                auto params = thread->template data<thread_params>();
                params->characters.erase(ch.id());
            }
        }

        if (after != nullptr)
        {
            auto thread = after->thread();
            if (thread != nullptr)
            {
                auto params = thread->template data<thread_params>();
                params->characters.insert({ch.id(), ch.shared_from_this_as<character>()});
            }
        }
    }
}