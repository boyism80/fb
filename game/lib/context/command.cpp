#include <fb/game/context.h>
#include <fb/core/redis.h>
using namespace fb::game;

fb::task<bool> fb::game::context::handle_command_map(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto map = fb::game::old_model::maps.name2map(name);
    if(map == nullptr)
        co_return false;

    auto x = 0;
    auto y = 0;
    if(parameters.size() == 3)
    {
        if(parameters[1].isNumeric())
            x = parameters[1].asInt();

        if(parameters[2].isNumeric())
            y = parameters[2].asInt();
    }
    co_await session.co_map(map, point16_t(x, y));
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_sound(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::sound(session, fb::game::SOUND_TYPE(value)), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_action(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::session::action(session, fb::game::ACTION_TYPE(value), DURATION::SPELL), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_weather(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::weather(WEATHER_TYPE(value)), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_bright(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::bright(value), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_timer(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::timer(value), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_effect(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::effect(session, value), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_disguise(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto mob = fb::game::old_model::mobs.name2mob(name);
    if(mob == nullptr)
        co_return true;

    session.disguise(mob->look);
    this->send(session, fb::protocol::game::response::object::effect(session, 0x03), scope::PIVOT);
    this->send(session, fb::protocol::game::response::session::action(session, ACTION_TYPE::CAST_SPELL, DURATION::SPELL), scope::PIVOT);
    this->send(session, fb::protocol::game::response::object::sound(session, SOUND_TYPE(0x0019)), scope::PIVOT);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_undisguise(fb::game::session& session, Json::Value& parameters)
{
    session.undisguise();
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_mob(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto core = fb::game::old_model::mobs.name2mob(name);
    if(core == nullptr)
        co_return true;

    auto mob = this->make<fb::game::mob>(core, fb::game::mob::config { .alive = true });
    auto map = session.map();
    co_await mob->co_map(map, session.position());
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_class(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    uint8_t cls, promotion;
    if(fb::game::old_model::classes.name2class(name, &cls, &promotion) == false)
        co_return true;

    session.cls(cls);
    session.promotion(promotion);
    this->send(session, fb::protocol::game::response::session::id(session), scope::SELF);
    this->send(session, fb::protocol::game::response::session::state(session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_level(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto level = parameters[0].asInt();
    session.level(level);
    this->send(session, fb::protocol::game::response::session::state(session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_spell(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto spell = fb::game::old_model::spells.name2spell(name);
    if(spell == nullptr)
        co_return false;

    auto slot = session.spells.add(spell);
    if(slot == 0xFF)
        co_return false;

    co_return true;
}

fb::task<bool> fb::game::context::handle_command_item(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto core = fb::game::old_model::items.name2item(name);
    if(core == nullptr)
        co_return false;

    auto count = parameters.size() > 1 && parameters[1].isInt() ? 
        parameters[1].asInt() : 1;

    auto item = core->make(*this, count);
    co_await item->co_map(session.map(), session.position());
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_world(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto id = parameters[0].asString();
    auto response = fb::protocol::game::response::map::worlds(id);
    if(response.offset == nullptr)
        co_return false;

    // session.map(nullptr);
    session.send(response);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_script(fb::game::session& session, Json::Value& parameters)
{
    session.dialog
        .from("scripts/script.lua")
        .func("func")
        .pushobject(session)
        .resume(1);

    co_return true;
}

fb::task<bool> fb::game::context::handle_command_hair(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isInt() == false)
        co_return false;

    auto hair = parameters[0].asInt();
    session.look(hair);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_hair_color(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isInt() == false)
        co_return false;

    auto color = parameters[0].asInt();
    session.color(color);
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_armor_color(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() == 0)
    {
        session.armor_color(std::nullopt);
        co_return true;
    }
    else if(parameters[0].isInt() == false)
    {
        co_return false;
    }
    else
    {
        auto color = parameters[0].asInt();
        session.armor_color(color);
        co_return true;
    }
}

fb::task<bool> fb::game::context::handle_command_exit(fb::game::session& session, Json::Value& parameters)
{
    this->_internal->send(fb::protocol::internal::request::shutdown());
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_tile(fb::game::session& session, Json::Value& parameters)
{
    auto listener = session.get_listener<fb::game::session>();
    auto map = session.map();
    if(map == nullptr)
        co_return false;
    
    auto tile = (*map)(session.x(), session.y());
    if(tile == nullptr)
        co_return false;

    auto sstream = std::stringstream();
    sstream << "맵타일 : " << tile->id;
    if(listener != nullptr)
        listener->on_notify(session, sstream.str());

    sstream.str("");
    sstream << "오브젝트 : " << tile->object;
    if(listener != nullptr)
        listener->on_notify(session, sstream.str());
    
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_save(fb::game::session& session, Json::Value& parameters)
{
    this->save();
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_mapobj(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();

    auto map = session.map();
    if(map == nullptr)
        co_return false;

    (*map)(session.x(), session.y())->object = value;
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_randmap(fb::game::session& session, Json::Value& parameters)
{
    static std::vector<fb::game::map*>  maps;
    static std::once_flag               flag;
    std::call_once(flag, [] 
        { 
            std::srand(std::time(nullptr));
            for(auto& [id, map] : fb::game::old_model::maps)
            {
                maps.push_back(map.get());
            }
        });

    auto index = std::rand() % maps.size();
    auto map = maps[index];
    auto x = map->width() > 0 ? std::rand() % map->width() : 0;
    auto y = map->height() > 0 ? std::rand() % map->height() : 0;

    co_await session.co_map(map, fb::game::point16_t(x, y));
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_npc(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();

    auto model = fb::game::old_model::npcs.name2npc(name);
    if(model == nullptr)
        co_return false;

    auto npc = model->make<fb::game::npc>(*this);
    npc->direction(session.direction());
    npc->map(session.map(), session.position());
    co_return true;
}

fb::task<bool> fb::game::context::handle_command_durability(fb::game::session& session, Json::Value& parameters)
{
    if(parameters.size() < 1)
        co_return false;

    if(parameters[0].isNumeric() == false)
        co_return false;

    auto percent = std::max(0, std::min(100, (int)parameters[0].asUInt()));
    auto equipments = std::vector<fb::game::equipment*>();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        if (item->attr(fb::game::item::ATTRIBUTE::EQUIPMENT) == false)
            continue;

        auto equipment = static_cast<fb::game::equipment*>(item);
        equipments.push_back(equipment);
    }

    for (auto& [parts, item] : session.items.equipments())
    {
        if (item == nullptr)
            continue;

        auto equipment = static_cast<fb::game::equipment*>(item);
        equipments.push_back(equipment);
    }

    for (auto equipment : equipments)
    {
        auto model = equipment->based<fb::game::equipment>();
        equipment->durability(uint16_t(model->durability * (percent / 100.0f)));
    }

    co_return true;
}

fb::task<bool> fb::game::context::handle_concurrency(fb::game::session& session, Json::Value& parameters)
{
    auto seconds = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 10;
    auto key = parameters.size() >= 2 && parameters[1].isString() ? parameters[1].asString() : "global";

    try
    {
        co_await this->_mutex.try_sync<bool>(key, [this, &session, seconds]() -> fb::task<bool>
        {
            for (int i = 0; i < seconds; i++)
            {
                session.chat(fb::format("%d초 후에 풀립니다.", seconds - i));
                co_await this->sleep(1s);
            }
            co_return true;
        });
    }
    catch(fb::lock_error&)
    {
        session.chat("리소스 점유 실패");
    }
    catch(std::exception& e)
    {
        session.chat(e.what());
    }

    co_return true;
}