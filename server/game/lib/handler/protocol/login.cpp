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

async::task<bool>
login::init_ch(const internal::Character& response, character& ch, std::optional<uint32_t> group, std::optional<uint32_t> clan, const std::optional<transfer_param>& transfer)
{
    auto map  = response.map;
    auto weak = ch.weak_from_this_as<character>();
    ch.id(response.id);
    ch.name(response.name);
    ch.pw(response.pw);
    ch.birthday(response.birth);
    ch.created_date(fb::model::datetime(response.created_date));
    ch.updated_date(fb::model::datetime(response.updated_date));
    ch.role(static_cast<ROLE>(response.role));
    ch.cls(static_cast<CLASS>(response.class_type));
    ch.promotion(response.promotion);
    ch.color(response.color);
    ch.direction(DIRECTION(response.direction));
    ch.look(response.look);
    ch.money(response.money);
    ch.items.deposited(response.deposited_money);
    ch.sex(SEX(response.sex));
    ch.stat.base_hp(response.base_hp);
    ch.stat.hp(response.hp);
    ch.stat.base_mp(response.base_mp);
    ch.stat.mp(response.mp);
    ch.level(response.level);
    ch.exp(response.exp);
    ch.state(STATE(response.state));
    ch.title(response.title);

    if (response.armor_color.has_value())
        ch.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        ch.disguise(response.disguise.value());
    else
        ch.undisguise();

    for (auto& buff : response.buffs)
    {
        auto& model = table::spell[buff.model];
        ch.buffs.push_back(model, buff.time);
    }

    if (this->server.maps.contains(map) == false)
        co_return false;

    auto position_x = response.position.x;
    auto position_y = response.position.y;
    if (transfer != std::nullopt)
    {
        map        = transfer.value().map;
        position_x = uint32_t(transfer.value().position.x);
        position_y = uint32_t(transfer.value().position.y);
    }

    if (group.has_value())
    {
        std::ignore = this->server.upsert_group_then(group.value(), [&ch](auto& group) {
            group->enter(ch.weak_from_this_as<character>());
            ch.group_id(group->id());
        });
    }

    if (clan.has_value())
    {
        co_await this->server.upsert_clan_then(clan.value(), [weak](auto& clan) -> async::task<void> {
            auto ch = weak.lock();
            if (ch == nullptr)
                co_return;

            clan->attach_character(weak);
            ch->clan_id(clan->id());
            co_return;
        });
    }

    co_return co_await ch.map(this->server.maps[map], fb::model::point16_t(position_x, position_y));
}

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

void login::init_system_mail_users(const std::vector<fb::protocol::internal::SystemMailUser>& response, fb::game::character& ch)
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

std::string login::elapsed_message(const std::string& dt)
{
    auto elapsed = fb::model::datetime() - fb::model::datetime(dt);
    if (elapsed.total_milliseconds() < 1000 * 60)
        return std::string();

    auto sstream = std::stringstream();
    auto days    = elapsed.days();
    if (days > 0)
        sstream << days << "일 ";

    auto hours = elapsed.hours();
    if (hours > 0)
        sstream << hours << "시간 ";

    auto minutes = elapsed.minutes();
    if (minutes > 0)
        sstream << minutes << "분";
    sstream << "만에 바람으로...";

    return sstream.str();
}

async::task<void> login::force_disconnect_and_wait(const std::string& name, const std::string& log_context)
{
    auto existing_ch = this->server.characters.find(name);
    if (existing_ch == nullptr)
        co_return;

    fb::logger::warn("Character {} {} - forcing disconnect", name, log_context);
    auto existing_socket = existing_ch->socket();
    if (existing_socket != nullptr)
        existing_socket->close();

    // Wait for character to be completely removed (max 5 seconds)
    auto timeout = std::chrono::steady_clock::now() + 5s;
    while (this->server.characters.contains(name))
    {
        if (std::chrono::steady_clock::now() >= timeout)
        {
            fb::logger::warn("Timeout waiting for character {} to be removed ({})", name, log_context);
            break;
        }
        co_await this->server.sleep(100ms);
    }
}

async::task<bool> login::handle(fb::socket<character>& session, fb::protocol::game::request::login& request)
{
    auto ch = session.data();
    if (ch == nullptr)
        co_return false;

    auto weak = ch->weak_from_this();
    session.encryption(request.enc_type, request.enc_key);

    ch->name(request.name);
    fb::logger::info("{} has connected.", request.name);

    auto delay = fb::config<uint32_t>("delay");
    co_await this->server.sleep(std::chrono::seconds(delay));

    // Check if character already exists in server before login API call
    co_await this->force_disconnect_and_wait(request.name, "already exists in server before new login");

    auto&& login_resp = co_await this->server.http.post("internal", "/in-game/login", Login{request.id, request.name, fb::config<uint8_t>("id")});
    if (weak.expired())
        co_return false;

    co_await this->server.threads.switching(weak);
    switch (static_cast<ERROR_CODE>(login_resp.error))
    {
    case ERROR_CODE::NONE:
        // Additional safety check: session is not in Redis but character exists in server (abnormal state)
        co_await this->force_disconnect_and_wait(request.name, "exists in server but session not in Redis");
        break;
    case ERROR_CODE::BANNED:
        ch->message(fb::game::handler::amqp::ban::build_ban_message(login_resp.ban_reason, login_resp.ban_expire_date), MESSAGE_TYPE::NOTIFY);
        co_return false;
    default:
        fb::logger::fatal("Unknown error: {}", login_resp.error);
        co_return false;
    }

    auto&& response = co_await this->server.http.get<internal_resp::Init>("internal", std::format("/in-game/init/{}", request.id));
    auto   map      = request.transfer.has_value() ? request.transfer->map : response.character.map;
    if (weak.expired())
        co_return false;

    ch->thread(this->server.maps[map]->thread());
    co_await this->server.threads.switching(weak);

    if (co_await this->init_ch(response.character, *ch, response.group, response.clan, request.transfer) == false)
        co_return false;
    co_await this->server.threads.switching(weak);

    ch->mail_box.unread_count(response.mail);

    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_achievements(response.achievements, *ch);
    this->init_quests(response.quests, *ch);
    this->init_system_mail_users(response.received_system_mails, *ch);

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

    ch->storage_box.init(storage_boxes, storage_reward_marks);

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

        ch->storage_box.apply_pending(pending_models);
    }

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
    this->server.characters.insert(ch->shared_from_this_as<character>());
    co_await ch->process_system_mails();

    // Log login event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch->id());
    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
    log_data["level"]          = ch->level();
    log_data["map"]            = ch->map()->model.id;
    log_data["position_x"]     = ch->position().x;
    log_data["position_y"]     = ch->position().y;
    this->server.log.write("login", log_data);

    co_return true;
}
