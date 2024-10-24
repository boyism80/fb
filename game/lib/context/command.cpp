#include <context.h>
#include <fb/redis.h>
using namespace fb::game;

async::task<bool> fb::game::context::handle_command_map(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.map.name2map(name);
    if (model == nullptr)
        co_return false;

    auto x = 0;
    auto y = 0;
    if (parameters.size() == 3)
    {
        if (parameters[1].isNumeric())
            x = parameters[1].asInt();

        if (parameters[2].isNumeric())
            y = parameters[2].asInt();
    }

    auto& map = this->maps[model->id];
    co_await session.map(&map, point16_t(x, y));
    co_return true;
}

async::task<bool> fb::game::context::handle_command_sound(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::sound(session, SOUND(value)), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_action(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::session::action(session, ACTION(value), DURATION::SPELL), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_weather(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::weather(WEATHER_TYPE(value)), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_bright(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::bright(value), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_timer(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::timer(value), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_effect(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    this->send(session, fb::protocol::game::response::object::effect(session, value), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_disguise(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto mob  = this->model.mob.name2mob(name);
    if (mob == nullptr)
        co_return true;

    session.disguise(mob->look);
    this->send(session, fb::protocol::game::response::object::effect(session, 0x03), scope::PIVOT);
    this->send(session, fb::protocol::game::response::session::action(session, ACTION::CAST_SPELL, DURATION::SPELL), scope::PIVOT);
    this->send(session, fb::protocol::game::response::object::sound(session, SOUND::DISGUISE), scope::PIVOT);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_undisguise(fb::game::character& session, Json::Value& parameters)
{
    session.undisguise();
    co_return true;
}

async::task<bool> fb::game::context::handle_command_mob(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.mob.name2mob(name);
    if (model == nullptr)
        co_return true;

    auto mob = model->make<fb::game::mob>(*this, fb::game::mob::config{.alive = true});
    auto map = session.map();
    co_await mob->map(map, session.position());
    co_return true;
}

async::task<bool> fb::game::context::handle_command_class(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name       = parameters[0].asString();
    auto class_type = CLASS::NONE;
    auto promotion  = uint8_t(0);
    if (this->model.promotion.name2class(name, class_type, promotion) == false)
        co_return true;

    session.cls(class_type);
    session.promotion(promotion);
    this->send(session, fb::protocol::game::response::session::id(session), scope::SELF);
    this->send(session, fb::protocol::game::response::session::state(session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_level(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto level = parameters[0].asInt();
    session.level(level);
    this->send(session, fb::protocol::game::response::session::state(session, STATE_LEVEL::LEVEL_MAX), scope::SELF);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_spell(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.spell.name2spell(name);
    if (model == nullptr)
        co_return false;

    auto slot = co_await session.spells.add(*model);
    if (slot == 0xFF)
        co_return false;

    co_return true;
}

async::task<bool> fb::game::context::handle_command_item(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.item.name2item(name);
    if (model == nullptr)
        co_return false;

    auto count = parameters.size() > 1 && parameters[1].isInt() ? parameters[1].asInt() : 1;

    auto item  = model->make(*this, count);
    co_await item->map(session.map(), session.position());
    co_return true;
}

async::task<bool> fb::game::context::handle_command_world(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    for (auto& [id, world] : this->model.world)
    {
        for (auto& [index, point] : world)
        {
            if (point.name == name)
            {
                session.send(fb::protocol::game::response::map::worlds(this->model, id, index));
                co_return true;
            }
        }
    }

    co_return false;
}

async::task<bool> fb::game::context::handle_command_script(fb::game::character& session, Json::Value& parameters)
{
    session.dialog.from("scripts/script.lua").func("func").pushobject(session).resume(1);

    co_return true;
}

async::task<bool> fb::game::context::handle_command_hair(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isInt() == false)
        co_return false;

    auto hair = parameters[0].asInt();
    session.look(hair);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_hair_color(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isInt() == false)
        co_return false;

    auto color = parameters[0].asInt();
    session.color(color);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_armor_color(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() == 0)
    {
        session.armor_color(std::nullopt);
        co_return true;
    }
    else if (parameters[0].isInt() == false)
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

async::task<bool> fb::game::context::handle_command_exit(fb::game::character& session, Json::Value& parameters)
{
    // this->_internal->send(fb::protocol::internal::request::shutdown());
    co_return true;
}

async::task<bool> fb::game::context::handle_command_tile(fb::game::character& session, Json::Value& parameters)
{
    auto listener = session.get_listener<fb::game::character>();
    auto map      = session.map();
    if (map == nullptr)
        co_return false;

    auto tile = (*map)(session.x(), session.y());
    if (tile == nullptr)
        co_return false;

    auto sstream = std::stringstream();
    sstream << "맵타일 : " << tile->id;
    if (listener != nullptr)
        listener->on_notify(session, sstream.str());

    sstream.str("");
    sstream << "오브젝트 : " << tile->object;
    if (listener != nullptr)
        listener->on_notify(session, sstream.str());

    co_return true;
}

async::task<bool> fb::game::context::handle_command_save(fb::game::character& session, Json::Value& parameters)
{
    co_await this->save(session);
    co_return true;
}

async::task<bool> fb::game::context::handle_command_mapobj(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();

    auto map   = session.map();
    if (map == nullptr)
        co_return false;

    (*map)(session.x(), session.y())->object = value;
    co_return true;
}

async::task<bool> fb::game::context::handle_command_randmap(fb::game::character& session, Json::Value& parameters)
{
    static std::vector<fb::game::map*> maps;
    static std::once_flag              flag;
    std::call_once(flag, [this] {
        std::srand(static_cast<uint32_t>(std::time(nullptr)));
        for (auto& [id, map] : this->maps)
        {
            maps.push_back(&map);
        }
    });

    auto  index = std::rand() % maps.size();
    auto& map   = maps[index];
    auto  x     = map->width() > 0 ? std::rand() % map->width() : 0;
    auto  y     = map->height() > 0 ? std::rand() % map->height() : 0;

    co_await session.map(map, point16_t(x, y));
    co_return true;
}

async::task<bool> fb::game::context::handle_command_npc(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();

    auto model = this->model.npc.name2npc(name);
    if (model == nullptr)
        co_return false;

    auto npc = model->make<fb::game::npc>(*this);
    npc->direction(session.direction());
    co_await npc->map(session.map(), session.position());
    co_return true;
}

async::task<bool> fb::game::context::handle_command_durability(fb::game::character& session, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto percent    = std::max(0, std::min(100, (int)parameters[0].asUInt()));
    auto equipments = std::vector<fb::game::equipment*>();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT) == false)
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
        auto& model = equipment->based<fb::model::equipment>();
        equipment->durability(uint16_t(model.durability * (percent / 100.0f)));
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_command_concurrency(fb::game::character& session, Json::Value& parameters)
{
    auto seconds = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 10;
    auto key     = parameters.size() >= 2 && parameters[1].isString() ? parameters[1].asString() : "global";

    try
    {
        co_await this->_mutex.sync<bool>(key, [this, &session, seconds](auto& trans) -> async::task<bool> {
            for (int i = 0; i < seconds; i++)
            {
                session.chat(std::format("{}초 후에 풀립니다.", seconds - i));
                co_await this->sleep(1s);
            }
            co_return true;
        });
    }
    catch (fb::lock_error&)
    {
        session.chat("리소스 점유 실패");
    }
    catch (std::exception& e)
    {
        session.chat(e.what());
    }

    co_return true;
}

async::task<bool> fb::game::context::handle_command_sleep(fb::game::character& session, Json::Value& parameters)
{
    auto seconds = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 10;
    co_await this->sleep(std::chrono::seconds{seconds});
    co_return true;
}