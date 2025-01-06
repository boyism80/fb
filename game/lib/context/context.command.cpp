#include <fb/game/context.h>
using namespace fb::game;

async::task<bool> context::handle_command_map(character& ch, Json::Value& parameters)
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

    auto& map   = this->maps[model->id];
    std::ignore = co_await ch.map(&map, fb::model::point16_t(x, y));
    co_return true;
}

async::task<bool> context::handle_command_sound(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.sound(SOUND(value));
    co_return true;
}

async::task<bool> context::handle_command_action(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.action(ACTION(value), DURATION::SPELL);
    co_return true;
}

async::task<bool> context::handle_command_weather(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.weather(WEATHER_TYPE(value));
    co_return true;
}

async::task<bool> context::handle_command_bright(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.bright(value);
    co_return true;
}

async::task<bool> context::handle_command_timer(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.timer(value, TIMER_TYPE::DECREASE);
    co_return true;
}

async::task<bool> context::handle_command_effect(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();
    ch.effect(value);
    co_return true;
}

async::task<bool> context::handle_command_disguise(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();
    auto mob  = this->model.mob.name2mob(name);
    if (mob == nullptr)
        co_return true;

    ch.disguise(mob->look);
    ch.effect(0x03);
    ch.action(ACTION::CAST_SPELL, DURATION::SPELL);
    ch.sound(SOUND::DISGUISE);
    co_return true;
}

async::task<bool> context::handle_command_undisguise(character& ch, Json::Value& parameters)
{
    ch.undisguise();
    co_return true;
}

async::task<bool> context::handle_command_mob(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.mob.name2mob(name);
    if (model == nullptr)
        co_return true;

    auto mob    = model->make<fb::game::mob>(*this, fb::game::mob::initial_params{.alive = true});
    auto map    = ch.map();
    std::ignore = co_await mob->map(map, ch.position());
    co_return true;
}

async::task<bool> context::handle_command_class(character& ch, Json::Value& parameters)
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

    ch.cls(class_type);
    ch.promotion(promotion);
    co_return true;
}

async::task<bool> context::handle_command_level(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto level = parameters[0].asInt();
    ch.level(level);
    co_return true;
}

async::task<bool> context::handle_command_hp(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto hp = parameters[0].asUInt();
    ch.base_hp(hp);
    ch.hp(hp);
    if (ch.state() == STATE::GHOST)
        ch.state(STATE::NORMAL);

    co_return true;
}

async::task<bool> context::handle_command_mp(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto mp = parameters[0].asUInt();
    ch.base_mp(mp);
    ch.mp(mp);
    co_return true;
}

async::task<bool> context::handle_command_spell(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name  = parameters[0].asString();
    auto model = this->model.spell.name2spell(name);
    if (model == nullptr)
        co_return false;

    auto slot = ch.spells.add(*model);
    if (slot == 0xFF)
        co_return false;

    co_return true;
}

async::task<bool> context::handle_command_item(character& ch, Json::Value& parameters)
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

    auto item   = model->make(*this, count);
    std::ignore = co_await item->map(ch.map(), ch.position());
    co_return true;
}

async::task<bool> context::handle_command_world(character& ch, Json::Value& parameters)
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
                ch.show_world_map(id, index);
                co_return true;
            }
        }
    }

    co_return false;
}

async::task<bool> context::handle_command_script(character& ch, Json::Value& parameters)
{
    ch.dialog.from("scripts/script.lua").func("func").pushobject(ch).resume(1);

    co_return true;
}

async::task<bool> context::handle_command_hair(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isInt() == false)
        co_return false;

    auto hair = parameters[0].asInt();
    ch.look(hair);
    co_return true;
}

async::task<bool> context::handle_command_hair_color(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isInt() == false)
        co_return false;

    auto color = parameters[0].asInt();
    ch.color(color);
    co_return true;
}

async::task<bool> context::handle_command_armor_color(character& ch, Json::Value& parameters)
{
    if (parameters.size() == 0)
    {
        ch.armor_color(std::nullopt);
        co_return true;
    }
    else if (parameters[0].isInt() == false)
    {
        co_return false;
    }
    else
    {
        auto color = parameters[0].asInt();
        ch.armor_color(color);
        co_return true;
    }
}

async::task<bool> context::handle_command_exit(character& ch, Json::Value& parameters)
{
    // this->_internal->send(fb::protocol::internal::request::shutdown());
    co_return true;
}

async::task<bool> context::handle_command_tile(character& ch, Json::Value& parameters)
{
    auto listener = ch.get_listener<character>();
    auto map      = ch.map();
    if (map == nullptr)
        co_return false;

    auto tile = (*map)(ch.x(), ch.y());
    if (tile == nullptr)
        co_return false;

    auto sstream = std::stringstream();
    sstream << "맵타일 : " << tile->id;
    ch.message(sstream.str());

    sstream.str("");
    sstream << "오브젝트 : " << tile->object;
    ch.message(sstream.str());

    co_return true;
}

async::task<bool> context::handle_command_save(character& ch, Json::Value& parameters)
{
    for (int i = 0; i < this->threads.size(); i++)
    {
        std::ignore = this->threads[i]->dispatch([this](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& [id, character] : params->characters)
            {
                std::ignore = this->save(*character);
            }
            co_return;
        });
    }
    co_return true;
}

async::task<bool> context::handle_command_mapobj(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto value = parameters[0].asInt();

    auto map = ch.map();
    if (map == nullptr)
        co_return false;

    (*map)(ch.x(), ch.y())->object = value;
    co_return true;
}

async::task<bool> context::handle_command_randmap(character& ch, Json::Value& parameters)
{
    static std::vector<map*> maps;
    static std::once_flag    flag;
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

    co_return co_await ch.map(map, fb::model::point16_t(x, y));
}

async::task<bool> context::handle_command_npc(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isString() == false)
        co_return false;

    auto name = parameters[0].asString();

    auto model = this->model.npc.name2npc(name);
    if (model == nullptr)
        co_return false;

    auto npc    = model->make<fb::game::npc>(*this);
    std::ignore = npc->direction(ch.direction());
    co_return co_await npc->map(ch.map(), ch.position());
}

async::task<bool> context::handle_command_durability(character& ch, Json::Value& parameters)
{
    if (parameters.size() < 1)
        co_return false;

    if (parameters[0].isNumeric() == false)
        co_return false;

    auto percent    = std::max(0, std::min(100, (int)parameters[0].asUInt()));
    auto equipments = std::vector<equipment*>();
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = ch.items[i];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT) == false)
            continue;

        auto equipment = static_cast<fb::game::equipment*>(item);
        equipments.push_back(equipment);
    }

    for (auto& [parts, item] : ch.items.equipments())
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

async::task<bool> context::handle_command_concurrency(character& ch, Json::Value& parameters)
{
    auto seconds = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 10;
    auto key     = parameters.size() >= 2 && parameters[1].isString() ? parameters[1].asString() : "global";
    auto error   = std::string();
    try
    {
        co_await this->_mutex.sync<void>(key, [this, &ch, seconds](auto& trans) -> async::task<void> {
            for (int i = 0; i < seconds; i++)
            {
                ch.chat(std::format("{}초 후에 풀립니다.", seconds - i));
                co_await this->sleep(1s);
            }
        });
    }
    catch (fb::lock_error&)
    {
        error = "리소스 점유 실패";
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    ch.chat(error);
    co_return true;
}

async::task<bool> context::handle_command_sleep(character& ch, Json::Value& parameters)
{
    auto seconds = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 10;
    co_await this->sleep(std::chrono::seconds{seconds});
    co_return true;
}

async::task<bool> context::handle_map_tile(character& ch, Json::Value& parameters)
{
    auto map = ch.map();
    if (map == nullptr)
        co_return false;

    auto& position = ch.position();
    auto  tile     = (*map)(position.x, position.y);
    if (tile == nullptr)
        co_return false;

    ch.message(std::format("id: {}, object: {}, block: {}", tile->id, tile->object, tile->blocked));
    co_return true;
}

async::task<bool> context::handle_command_ad(character& ch, Json::Value& parameters)
{
    auto width  = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 300;
    auto height = parameters.size() >= 2 && parameters[1].isNumeric() ? parameters[1].asInt() : 120;
    auto url    = parameters.size() >= 3 && parameters[2].isString() ? parameters[2].asString()
                                                                     : std::string{"https://www.google.com"};
    auto time   = parameters.size() >= 4 && parameters[3].isNumeric() ? parameters[3].asInt() : 60;

    ch.send(fb::protocol::game::response::ad(width, height, url, time));
    co_return true;
}

async::task<bool> context::handle_command_web(character& ch, Json::Value& parameters)
{
    auto type = parameters.size() >= 1 && parameters[0].isNumeric() ? parameters[0].asInt() : 0;
    auto url  = parameters.size() >= 2 && parameters[1].isString() ? parameters[1].asString()
                                                                   : std::string{"https://www.google.com"};
    auto message =
        parameters.size() >= 3 && parameters[2].isString() ? parameters[2].asString() : std::string{"default message"};

    ch.send(fb::protocol::game::response::web(type, url, message));
    co_return true;
}

async::task<bool> context::handle_command_write_mail(character& ch, Json::Value& parameters)
{
    auto to    = parameters.size() >= 1 && parameters[0].isString() ? parameters[0].asString() : ch.name();
    auto count = parameters.size() >= 2 && parameters[1].isNumeric() ? parameters[1].asInt() : 1;
    auto fd    = ch.fd();
    try
    {
        for (int i = 0; i < count; i++)
        {
            auto title    = std::format("MAIL TITLE {}", i);
            auto contents = std::format("MAIL CONTENTS {}", i);
            co_await this->send_mail(ch, to, title, contents);
            co_await this->update_thread(ch);
        }
    }
    catch (std::exception& e)
    {
        if (this->alive(ch))
            ch.message(e.what());
    }

    co_return true;
}

async::task<bool> context::handle_command_read_mail(character& ch, Json::Value& parameters)
{
    auto fd = ch.fd();
    try
    {
        auto&& resp = co_await this->mail_list(ch, 0xFFFF, 0xFFFF);
        co_await this->update_thread(ch);

        this->assert_mail(resp.error);

        auto i = 0;
        for (auto& mail : resp.summary_list)
        {
            auto&& resp = co_await this->read_mail(ch, mail.id);
            co_await this->update_thread(ch);
            i++;
        }

        ch.message(std::format("{}개의 메일 읽음 처리", i));
    }
    catch (std::exception& e)
    {
        if (this->alive(ch))
            ch.message(e.what());
    }

    co_return true;
}

async::task<bool> context::handle_command_delete_mail(character& ch, Json::Value& parameters)
{
    auto fd = ch.fd();
    try
    {
        auto&& resp = co_await this->mail_list(ch, 0xFFFF, 0xFFFF);
        co_await this->update_thread(ch);

        this->assert_mail(resp.error);

        auto i = 0;
        for (auto& mail : resp.summary_list)
        {
            auto&& resp = co_await this->delete_mail(ch, mail.id);
            co_await this->update_thread(ch);
            i++;
        }

        ch.message(std::format("{}개의 메일 삭제", i));
    }
    catch (std::exception& e)
    {
        if (this->alive(ch))
            ch.message(e.what());
    }

    co_return true;
}