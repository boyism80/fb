#include <fb/game/handler/protocol/login.h>
#include <fb/game/server.h>
#include <fb/game/handler/amqp/ban.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <json/json.h>
#include <sstream>
#include <chrono>

using namespace fb::game::handler::protocol;
using table = fb::model::table;

login::login(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::login>(server)
{ }

void login::init_option(const internal::Option& response, fb::game::character& ch)
{
    ch.option(OPTION::WHISPER, response.whisper, false);
    ch.option(OPTION::GROUP, response.group, false);
    ch.option(OPTION::ROAR, response.roar, false);
    ch.option(OPTION::ROAR_WORLDS, response.roar_worlds, false);
    ch.option(OPTION::MAGIC_EFFECT, response.magic_effect, false);
    ch.option(OPTION::WEATHER_EFFECT, response.weather_effect, false);
    ch.option(OPTION::FIXED_MOVE, response.fixed_move, false);
    ch.option(OPTION::TRADE, response.trade, false);
    ch.option(OPTION::FAST_MOVE, response.fast_move, false);
    ch.option(OPTION::EFFECT_SOUND, response.effect_sound, false);
    ch.option(OPTION::PK_PROTECT, response.pk_protect, false);
}

void login::init_items(const std::vector<internal::Item>& response, character& ch)
{
    for (auto& x : response)
    {
        auto item = table::item[x.model].make(this->server);
        item->count(x.count);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.stored != -1)
            ch.items.store(item);
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            ch.items.add(item, x.index);
        else
            ch.items.wear((EQUIPMENT_PARTS)x.parts, std::static_pointer_cast<fb::game::equipment>(item));

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<weapon*>(item.get())->custom_name(x.custom_name.value());
    }
}

void login::init_spells(const std::vector<internal::Spell>& response, character& ch)
{
    for (auto& x : response)
    {
        if (table::spell.contains(x.model) == false)
            continue;

        auto& model = table::spell[x.model];
        auto  delay = fb::model::datetime(x.next) - fb::model::datetime();
        auto  sec   = delay.seconds();
        if (sec >= 0)
            sec += (delay.milliseconds() > 0 ? 1 : 0);
        else
            sec = 0;
        ch.spells.add(model, x.slot, sec);
    }
}

void login::init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch)
{
    for (auto& x : response)
    {
        ch.quests.add(x.qid, x.step, x.progress, x.completed, x.param);
    }
}

void login::init_achievements(const std::vector<fb::protocol::internal::Achievement>& response, fb::game::character& ch)
{
    for (auto& achievement : response)
    {
        if (table::achievement.contains(achievement.model) == false)
            continue;

        auto ptr = std::make_unique<fb::game::achievement>(table::achievement[achievement.model], achievement.text, achievement.icon, achievement.color);
        ch.achievements.insert({achievement.model, std::move(ptr)});
    }
}

void login::init_system_mail(const std::vector<fb::protocol::internal::SystemMailUser>& response, fb::game::character& ch)
{
    for (auto& smu : response)
    {
        ch.mail_box.add_system_mail_user(smu.mail_id, smu.expire_date.has_value() ? std::make_optional(smu.expire_date.value()) : std::nullopt);
        if (smu.read)
        {
            ch.mail_box.update_system_mail_user_read(smu.mail_id, true);
        }
    }
}

void login::init_storage(const fb::protocol::internal::response::Init& response, fb::game::character& ch)
{
    auto storage_boxes = std::vector<fb::game::storage_box::entry>();
    storage_boxes.reserve(response.storage_boxes.size());
    for (const auto& dto : response.storage_boxes)
    {
        fb::game::storage_box::entry box{};
        box.id      = dto.id;
        box.title   = dto.title;
        box.message = dto.message;

        if (!dto.attachments.empty())
        {
            Json::Value        json;
            Json::Reader       reader;
            std::istringstream stream(dto.attachments);
            if (reader.parse(stream, json) && json.isArray())
            {
                box.attachments.reserve(json.size());
                for (const auto& item : json)
                {
                    box.attachments.emplace_back(item);
                }
            }
        }

        box.received = dto.received;
        if (dto.expired_date.has_value())
            box.expire_date = fb::model::datetime(dto.expired_date.value());
        storage_boxes.push_back(std::move(box));
    }

    auto storage_reward_marks = std::vector<fb::game::storage_box::reward_mark>();
    storage_reward_marks.reserve(response.storage_reward_marks.size());
    for (const auto& dto : response.storage_reward_marks)
    {
        fb::game::storage_box::reward_mark mark{};
        mark.user       = dto.user;
        mark.pending_id = dto.pending_id;
        if (dto.expired_date.has_value())
            mark.expire_date = fb::model::datetime(dto.expired_date.value());
        storage_reward_marks.push_back(std::move(mark));
    }

    ch.storage_box.init(storage_boxes, storage_reward_marks);

    if (response.storage_pending.empty() == false)
    {
        auto pending_models = std::vector<fb::game::storage_box::pending_box>();
        pending_models.reserve(response.storage_pending.size());
        for (const auto& dto : response.storage_pending)
        {
            fb::game::storage_box::pending_box pending_box{};
            pending_box.id      = dto.id;
            pending_box.user    = dto.user;
            pending_box.title   = dto.title;
            pending_box.message = dto.message;

            if (!dto.attachments.empty())
            {
                Json::Value        json;
                Json::Reader       reader;
                std::istringstream stream(dto.attachments);
                if (reader.parse(stream, json) && json.isArray())
                {
                    pending_box.attachments.reserve(json.size());
                    for (const auto& item : json)
                    {
                        pending_box.attachments.emplace_back(item);
                    }
                }
            }

            if (dto.expired_date.has_value())
                pending_box.expire_date = fb::model::datetime(dto.expired_date.value());
            pending_models.push_back(std::move(pending_box));
        }

        ch.storage_box.apply_pending(pending_models);
    }
}

async::task<std::shared_ptr<character>> login::init(const fb::protocol::game::request::login& request, fb::socket<character>& session)
{
    auto&& response = co_await this->server.http.get<internal_resp::Init>("internal", std::format("/in-game/init/{}", request.id));
    auto   map      = request.transfer.has_value() ? request.transfer->map : response.character.map;

    auto params         = character::initial_params{.socket = session};
    params.id           = response.character.id;
    params.name         = response.character.name;
    params.pw           = response.character.pw;
    params.birthday     = response.character.birth;
    params.created_date = fb::model::datetime(response.character.created_date);
    params.updated_date = fb::model::datetime(response.character.updated_date);
    params.role         = static_cast<ROLE>(response.character.role);
    params.class_type   = static_cast<CLASS>(response.character.class_type);
    params.promotion    = response.character.promotion;
    params.color        = response.character.color;
    params.direction    = DIRECTION(response.character.direction);
    params.look         = response.character.look;
    params.money        = response.character.money;
    params.sex          = SEX(response.character.sex);
    params.level        = response.character.level;
    params.exp          = response.character.exp;
    params.state        = STATE(response.character.state);
    params.title        = response.character.title;
    params.armor_color  = response.character.armor_color;
    params.disguise     = response.character.disguise;

    auto ch   = this->server.make<character>(params);
    auto weak = ch->weak_from_this_as<character>();
    co_await this->server.threads.switching(weak);
    ch->items.deposited(response.character.deposited_money);
    ch->stat.base_hp(response.character.base_hp);
    ch->stat.hp(response.character.hp);
    ch->stat.base_mp(response.character.base_mp);
    ch->stat.mp(response.character.mp);

    auto thread = this->server.maps[map]->thread();
    ch->thread(thread);
    co_await thread->switching();

    auto position_x = response.character.position.x;
    auto position_y = response.character.position.y;
    if (request.transfer != std::nullopt)
    {
        map        = request.transfer.value().map;
        position_x = uint32_t(request.transfer.value().position.x);
        position_y = uint32_t(request.transfer.value().position.y);
    }

    if (co_await ch->map(this->server.maps[map], fb::model::point16_t(position_x, position_y)) == false)
        co_return nullptr;

    for (auto& buff : response.character.buffs)
    {
        auto& model = table::spell[buff.model];
        ch->buffs.push_back(model, buff.time);
    }

    if (response.group.has_value())
    {
        co_await this->server.ensure_group(response.group.value(), [this, &ch, weak](auto& group) -> async::task<void> {
            group->enter(weak);
            ch->group_id(group->id());
            co_return;
        });
    }

    if (response.clan.has_value())
    {
        co_await this->server.ensure_clan(response.clan.value(), [this, &ch, weak](auto& clan) -> async::task<void> {
            clan->attach(weak);
            ch->clan_id(clan->id());
            co_return;
        });
    }

    ch->mail_box.unread_count(response.mail);
    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_achievements(response.achievements, *ch);
    this->init_quests(response.quests, *ch);
    this->init_system_mail(response.received_system_mails, *ch);
    this->init_storage(response, *ch);
    this->init_option(response.option, *ch);
    ch->init();
    ch->update_time(this->server.time().hours());
    if (request.from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(response.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG | defined _DEBUG
            lua->load("scripts/interaction.lua");
#endif
            lua->func("on_login");
            lua->pushobject(ch);
            std::ignore = lua->call(1);
        }
    }

    ch->update(UPDATE_STATE_LEVEL::ALL);
    ch->update_option();
    co_await ch->process_system_mails();
    co_return ch;
}

async::task<bool> login::assert_login(const fb::protocol::game::request::login& request)
{
    auto&& resp = co_await this->server.http.post("internal", "/in-game/login", Login{request.id, request.name, fb::config<uint8_t>("id"), false});
    switch (static_cast<ERROR_CODE>(resp.error))
    {
    case ERROR_CODE::NONE:
        co_return true;

    case ERROR_CODE::BANNED:
        fb::logger::warn("Character {} is banned: {}", request.name, resp.ban_reason);
        co_return false;

    default:
        fb::logger::fatal("Unknown error: {}", resp.error);
        co_return false;
    }
}

std::string login::elapsed_message(const std::string& dt)
{
    auto elapsed = fb::model::datetime() - fb::model::datetime(dt);
    if (elapsed.total_milliseconds() < 1000 * 60)
        return std::string();

    auto sstream = std::stringstream();
    auto days    = elapsed.days();
    if (days > 0)
        sstream << days << _TEXT(MESSAGE_LOGIN_ELAPSED_DAYS);

    auto hours = elapsed.hours();
    if (hours > 0)
        sstream << hours << _TEXT(MESSAGE_LOGIN_ELAPSED_HOURS);

    auto minutes = elapsed.minutes();
    if (minutes > 0)
        sstream << minutes << _TEXT(MESSAGE_LOGIN_ELAPSED_MINUTES);
    sstream << _TEXT(MESSAGE_LOGIN_ELAPSED_SUFFIX);

    return sstream.str();
}

async::task<bool> login::ensure_character_insert(const std::weak_ptr<fb::game::character>& weak)
{
    auto ch = weak.lock();
    if (ch == nullptr)
        co_return false;

    auto success = this->server.characters.write([ch](auto& container) {
        auto& name   = ch->name();
        auto  old_ch = container.find(name);
        if (old_ch == nullptr)
        {
            container.insert(ch);
            return true;
        }

        old_ch->socket.close();

        return false;
    });

    if (success)
        co_return true;

    auto timeout = std::chrono::steady_clock::now() + 5s;
    while (true)
    {
        auto inserted = this->server.characters.write([ch](auto& container) {
            return container.insert(ch);
        });

        if (inserted)
            co_return true;

        if (std::chrono::steady_clock::now() >= timeout)
            break;

        co_await this->server.sleep(100ms);
    }

    co_return false;
}

async::task<bool> login::handle(fb::socket<character>& session, fb::protocol::game::request::login& request)
{
    session.encryption(request.enc_type, request.enc_key);
    fb::logger::info("{} has connected.", request.name);

    auto exists = this->server.characters.read([id = request.id](auto& container) {
        return container.find(id) != nullptr;
    });
    if (exists)
    {
        fb::logger::fatal("Character {} already exists in server during initial insert - disconnecting duplicate session", request.name);
        co_return false;
    }

    auto delay = fb::config<uint32_t>("delay");
    co_await this->server.sleep(std::chrono::seconds(delay));

    if (co_await this->assert_login(request) == false)
        co_return false;

    auto ch = co_await this->init(request, session);
    if (ch == nullptr)
        co_return false;

    auto inserted = this->server.characters.write([ch](auto& container) {
        return container.insert(ch);
    });

    if (inserted == false)
    {
        fb::logger::fatal("Character {} already exists in server during initial insert - disconnecting duplicate session", request.name);
        co_return false;
    }

    session.data(ch);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
    log_data["level"]          = ch->level();
    log_data["map"]            = ch->map()->model.id;
    log_data["position_x"]     = ch->position().x;
    log_data["position_y"]     = ch->position().y;
    this->server.log.write("login", log_data);

    co_return true;
}
