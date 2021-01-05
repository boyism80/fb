#include "model/acceptor/acceptor.game.h"
using namespace fb::game;

bool fb::game::acceptor::handle_command_map(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    auto map = fb::game::table::maps.name2map(name);
    if(map == nullptr)
        return false;

    auto x = 0;
    auto y = 0;
    if(parameters.size() > 3)
    {
        if(parameters[1].isNumeric())
            x = parameters[1].asInt();

        if(parameters[2].isNumeric())
            y = parameters[2].asInt();
    }
    session.map(map, point16_t(x, y));
    return true;
}

bool fb::game::acceptor::handle_command_sound(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::sound(session, fb::game::sound::type(value)), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_action(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::session::action(session, fb::game::action(value), duration::DURATION_SPELL), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_weather(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::weather(weather::type(value)), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_bright(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::bright(value), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_timer(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::timer(value), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_effect(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::effect(session, value), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_disguise(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    auto mob = fb::game::table::mobs.name2mob(name);
    if(mob == nullptr)
        return true;

    session.disguise(mob->look());
    this->send(session, fb::protocol::game::response::object::effect(session, 0x03), scope::PIVOT);
    this->send(session, fb::protocol::game::response::session::action(session, action::CAST_SPELL, duration::DURATION_SPELL), scope::PIVOT);
    this->send(session, fb::protocol::game::response::object::sound(session, sound::type(0x0019)), scope::PIVOT);
    return true;
}

bool fb::game::acceptor::handle_command_undisguise(fb::game::session& session, Json::Value& parameters)
{
    session.undisguise();
    return true;
}

bool fb::game::acceptor::handle_command_mob(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    auto core = fb::game::table::mobs.name2mob(name);
    if(core == nullptr)
        return true;

    auto mob = new fb::game::mob(core, this, true);
    auto map = session.map();
    mob->map(map, session.position());
    return true;
}

bool fb::game::acceptor::handle_command_class(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    uint8_t cls, promotion;
    if(fb::game::table::classes.name2class(name, &cls, &promotion) == false)
        return true;

    session.cls(cls);
    session.promotion(promotion);
    this->send(session, fb::protocol::game::response::session::id(session), scope::SELF);
    this->send(session, fb::protocol::game::response::session::state(session, state_level::LEVEL_MAX), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_command_level(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isNumeric() == false)
        return false;

    auto level = parameters[0].asInt();
    session.level(level);
    this->send(session, fb::protocol::game::response::session::state(session, state_level::LEVEL_MAX), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_command_spell(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    auto spell = fb::game::table::spells.name2spell(name);
    if(spell == nullptr)
        return false;

    auto slot = session.spells.add(spell);
    if(slot == 0xFF)
        return false;

    return true;
}

bool fb::game::acceptor::handle_command_item(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto name = parameters[0].asString();
    auto core = fb::game::table::items.name2item(name);
    if(core == nullptr)
        return false;

    auto item = core->make<fb::game::item>(this);
    item->map(session.map(), session.position());
    return true;
}

bool fb::game::acceptor::handle_command_world(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        return false;

    if(parameters[0].isString() == false)
        return false;

    auto id = parameters[0].asString();
    session.before_map(session.map());
    session.map(nullptr);
    session.send(fb::protocol::game::response::map::worlds(id));
    return true;
}

bool fb::game::acceptor::handle_command_script(fb::game::session& session, Json::Value& parameters)
{
    session.dialog
        .from("scripts/test.lua")
        .func("func")
        .pushobject(session)
        .resume(1);

    return true;
}