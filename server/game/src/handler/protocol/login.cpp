#include <fb/game/handler/protocol/login.h>
#include <fb/game/server.h>
#include <fb/amqp_route.h>
#include <fb/game/handler/amqp/ban.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <json/json.h>
#include <sstream>
#include <chrono>
#include <tuple>
#include <format>

using namespace fb::game;
using table = fb::model::table;

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
login<V>::login(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::login<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_option(const internal::Option& response, fb::game::character& ch)
{
    ch.option(OPTION::WHISPER, response.whisper, false);
    ch.option(OPTION::GROUP, response.group, false);
    ch.option(OPTION::ROAR, response.roar, false);
    ch.option(OPTION::NEWS, response.news, false);
    ch.option(OPTION::MAGIC_EFFECT, response.magic_effect, false);
    ch.option(OPTION::WEATHER_EFFECT, response.weather_effect, false);
    ch.option(OPTION::FIXED_MOVE, response.fixed_move, false);
    ch.option(OPTION::TRADE, response.trade, false);
    ch.option(OPTION::FAST_MOVE, response.fast_move, false);
    ch.option(OPTION::EFFECT_SOUND, response.effect_sound, false);
    ch.option(OPTION::PK_PROTECT, response.pk_protect, false);
    ch.option(OPTION::VISIBLE_HELMET, response.visible_helmet, false);
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_items(const std::vector<internal::Item>& response, character& ch)
{
    for (auto& x : response)
    {
        std::optional<fb::model::datetime> expire_time = std::nullopt;
        if (x.expire_time.has_value())
            expire_time = fb::model::datetime(x.expire_time.value());

        auto item = table::item[x.model].make(this->server, x.count, expire_time);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.stored != -1)
            std::ignore = ch.items.store(item);
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            std::ignore = ch.items.add(item, x.index);
        else
            std::ignore = ch.items.wear((EQUIPMENT_PARTS)x.parts, std::static_pointer_cast<fb::game::equipment>(item));

        if (x.custom_name.has_value() && item->model().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<weapon*>(item.get())->custom_name(x.custom_name.value());
    }
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_spells(const std::vector<internal::Spell>& response, character& ch)
{
    for (auto& x : response)
    {
        if (table::spell->contains(x.model) == false)
            continue;

        auto  spell_table = table::spell;
        auto& model       = spell_table[x.model];
        auto  delay       = fb::model::datetime(x.next) - this->server.now();
        auto  sec         = delay.seconds();
        if (sec >= 0)
            sec += (delay.milliseconds() > 0 ? 1 : 0);
        else
            sec = 0;
        std::ignore = ch.spells.add(model, x.slot, sec);
    }
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_matchmaker(const std::vector<internal::MatchmakingSkill>& response, character& ch)
{
    ch.matchmaker.load(response);
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch)
{
    for (auto& x : response)
    {
        ch.quests.add(x.qid, x.step, x.progress, x.completed, x.param);
    }
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_marketplace(const std::vector<fb::protocol::internal::MarketplacePending>& response,
                                fb::game::character&                                           ch)
{
    auto pending_listings = fb::game::marketplace::pending_listings_t{};
    for (const auto& row : response)
    {
        fb::game::marketplace::pending_listing_info info{};
        info.type                    = static_cast<fb::game::marketplace::pending_type>(row.type);
        info.purchase_id             = row.purchase_id;
        info.listing_id              = row.listing_id;
        info.character_id            = row.character_id;
        info.expected_purchase_count = row.expected_purchase_count;
        info.expected_total_price    = row.expected_total_price;

        auto reader = Json::Reader{};
        auto json   = Json::Value{};
        if (reader.parse(row.attachments, json) && json.isArray())
        {
            for (const auto& dsl_json : json)
            {
                info.dsls.push_back(fb::model::dsl(dsl_json));
            }
        }

        if (!info.dsls.empty())
            pending_listings[row.pending_key] = std::move(info);
    }

    if (!pending_listings.empty())
        ch.marketplace.set_pending_listings(std::move(pending_listings));
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_collection_unlocks(const std::vector<fb::protocol::internal::CollectionUnlock>& response,
                                       fb::game::character&                                         ch)
{
    ch.collections.load(response);
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_achievements(const std::vector<fb::protocol::internal::Achievement>& response,
                                 fb::game::character&                                    ch)
{
    for (auto& a : response)
    {
        auto ptr = std::make_unique<fb::game::achievement>(a.model, a.text, a.icon, a.color);
        ch.achievements.insert({a.model, std::move(ptr)});
    }
}

template <fb::protocol::CLIENT_VERSION V>
void login<V>::init_storage(const fb::protocol::internal::response::Init& response, fb::game::character& ch)
{
    ch.server.system_storage.init_from_login(ch, response.storage_boxes);
}

template <fb::protocol::CLIENT_VERSION V>
async::task<std::shared_ptr<character>> login<V>::init(const game_reqs::login<V>& request,
                                                       fb::socket<character>&     session)
{
    auto world = request.transfer.has_value() ? request.transfer->world : fb::config<uint32_t>("world");
    if (world == 0)
        world = fb::config<uint32_t>("world");
    auto&& resp = co_await this->server.http.template get<internal_resp::Init>(
        "internal",
        std::format("/in-game/init/{}/{}", world, request.id));
    auto map        = request.transfer.has_value() ? request.transfer->map : resp.character.map;
    auto position_x = resp.character.position.x;
    auto position_y = resp.character.position.y;
    if (request.transfer != std::nullopt)
    {
        map        = request.transfer.value().map;
        position_x = uint32_t(request.transfer.value().position.x);
        position_y = uint32_t(request.transfer.value().position.y);
    }
    else if (table::map->contains(map) && table::map[map].return_to.has_value())
    {
        auto source_map_id = map;
        auto return_map_id = table::map[map].return_to.value();
        if (this->server.maps.contains(return_map_id) == false)
        {
            fb::logger::fatal("Character {} login failed: return_to map {} does not exist (source map {})",
                              resp.character.name,
                              return_map_id,
                              source_map_id);
            co_return nullptr;
        }

        auto return_map = this->server.maps[return_map_id];
        if (return_map->active == false)
        {
            fb::logger::fatal("Character {} login failed: return_to map {} is not active (source map {})",
                              resp.character.name,
                              return_map_id,
                              source_map_id);
            co_return nullptr;
        }

        auto spawn = return_map->model().spawn_position().value_or(fb::model::point16_t{0, 0});
        map        = return_map_id;
        position_x = spawn.x;
        position_y = spawn.y;
    }

    auto socket_ptr     = session.shared_from_this_as<fb::socket<character>>();
    auto params         = character::initial_params{.socket = socket_ptr};
    params.id           = resp.character.id;
    params.world        = resp.character.world != 0 ? resp.character.world : world;
    params.name         = resp.character.name;
    params.pw           = resp.character.pw;
    params.birthday     = resp.character.birth;
    params.created_date = fb::model::datetime(resp.character.created_date);
    params.updated_date = fb::model::datetime(resp.character.updated_date);
    if (resp.character.first_login_date.has_value())
        params.first_login_date = std::make_optional(fb::model::datetime(resp.character.first_login_date.value()));
    else
        params.first_login_date = std::nullopt;
    params.role         = static_cast<ROLE>(resp.character.role);
    params.class_type   = static_cast<CLASS>(resp.character.class_type);
    params.promotion    = resp.character.promotion;
    params.color        = resp.character.color;
    params.direction    = static_cast<DIRECTION>(resp.character.direction);
    params.hair         = resp.character.hair;
    params.face         = resp.character.face;
    params.money        = resp.character.money;
    params.gender       = static_cast<GENDER>(resp.character.gender);
    params.level        = resp.character.level;
    params.exp          = resp.character.exp;
    params.state        = static_cast<STATE>(resp.character.state);
    params.title        = resp.character.title;
    params.armor_color  = resp.character.armor_color;
    params.weapon_color = resp.character.weapon_color;
    params.shield_color = resp.character.shield_color;
    if (resp.character.mimicry.has_value())
    {
        auto const& m  = resp.character.mimicry.value();
        auto state     = m.state.has_value() ? std::optional<STATE>(static_cast<STATE>(m.state.value())) : std::nullopt;
        params.mimicry = character_appearance<>(static_cast<GENDER>(m.gender),
                                                state,
                                                m.hair,
                                                m.hair_color,
                                                m.weapon,
                                                m.weapon_color,
                                                m.armor,
                                                m.armor_color,
                                                m.shield,
                                                m.shield_color,
                                                m.disguise);
    }
    else
    {
        params.mimicry = std::nullopt;
    }
    params.nation         = static_cast<NATION>(resp.character.nation);
    params.divine_beast   = static_cast<DIVINE_BEAST>(resp.character.divine_beast);
    params.super_hide     = resp.character.super_hide;
    params.client_version = request.client_version;
    params.ui_mode        = request.ui_mode;

    auto ch   = this->server.template make<character>(params);
    auto weak = ch->template weak_from_this_as<character>();
    co_await this->server.threads.switching(weak);
    if (fb::is_cross())
    {
        ch->restore_return_point(resp.character.map,
                                 fb::model::point16_t{static_cast<uint16_t>(resp.character.position.x),
                                                      static_cast<uint16_t>(resp.character.position.y)});
    }
    ch->items.deposited(resp.character.deposited_money);
    ch->stat.base_hp(resp.character.base_hp, false);
    ch->stat.hp(resp.character.hp, false);
    ch->stat.base_mp(resp.character.base_mp, false);
    ch->stat.mp(resp.character.mp, false);
    ch->stat.base_speed(resp.character.speed, false);

    auto thread = this->server.maps[map]->thread();
    ch->thread(thread);
    co_await thread->switching();

    session.data(ch);

    if (request.match.has_value() && request.match->id.empty() == false)
        co_await this->server.matches.join(*ch, request.match->id, request.match->type);

    if (ch->map() == nullptr)
    {
        if (co_await ch->map(this->server.maps[map], fb::model::point16_t(position_x, position_y)) == false)
        {
            this->server.matches.leave(*ch);
            co_return nullptr;
        }
    }

    for (auto& buff : resp.character.buffs)
    {
        auto  spell_table2 = table::spell;
        auto& model        = spell_table2[buff.model];
        std::ignore        = co_await ch->buffs.push_back(model, buff.time);
    }

    if (resp.group.has_value())
    {
        auto guard = co_await this->server.groups.ensure(resp.group.value());
        if (auto& group = guard.value(); group != nullptr)
        {
            group->enter(weak);
            ch->group_id(group->id());
            this->server.groups.update_portraits(*group);
        }
    }

    if (resp.clan.has_value())
    {
        auto guard = co_await this->server.clans.ensure(resp.clan.value());
        if (auto& clan = guard.value(); clan != nullptr)
        {
            clan->attach(weak);
            ch->clan_id(clan->id());
        }
    }

    bool inserted = this->server.characters.insert(ch);
    if (inserted == false)
    {
        this->server.matches.leave(*ch);
        fb::logger::fatal(
            "Character {} already exists in server during initial insert - disconnecting duplicate session",
            ch->name());
        co_return nullptr;
    }

    ch->mail_box.unread_count(resp.mail);
    ch->mail_box.init_system_mails(resp.system_mail_ids);
    this->init_items(resp.items, *ch);
    this->init_spells(resp.spells, *ch);
    this->init_matchmaker(resp.matchmaking_skills, *ch);
    this->init_achievements(resp.achievements, *ch);
    this->init_quests(resp.quests, *ch);
    this->init_collection_unlocks(resp.collection_unlocks, *ch);
    this->init_marketplace(resp.marketplace_pendings, *ch);
    this->init_storage(resp, *ch);
    this->init_option(resp.option, *ch);
    {
        auto entries = std::vector<friend_entry>{};
        entries.reserve(resp.friends.size());
        for (auto& entry : resp.friends)
            entries.push_back(friend_entry{entry.uid, entry.name, entry.mutual});
        ch->friends(std::move(entries));
    }
    ch->marriage(fb::game::marriage(resp.marriage.remarriage_after.empty()
                                        ? this->server.now()
                                        : fb::model::datetime(resp.marriage.remarriage_after),
                                    resp.marriage.spouse_id,
                                    resp.marriage.spouse_name,
                                    resp.marriage.divorce_count));

    ch->init();
    ch->collections.sync();
    ch->update_time(static_cast<uint8_t>(this->server.time().hours()),
                    static_cast<uint8_t>(this->server.time().minutes()));
    if (request.from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(resp.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);

        auto lua = this->server.lua.open("scripts/interaction.lua", "on_login");
        if (lua)
        {
            lua->pushobject(ch);
            lua->pushboolean(ch->is_first_login());
            std::ignore = lua->call(2);
        }

        ch->friends_sync(1);
        // Notify from Init mutual flags. Client may later replace the list;
        // accept that race rather than deferring behind a pending flag.
        {
            auto message = std::format(_TEXT(MESSAGE_FRIEND_LOGIN), ch->name());
            co_await ch->broadcast_friends(message, MESSAGE_TYPE::NOTIFY, true);
        }
    }

    co_await this->server.system_storage.sync(*ch);

    ch->update(UPDATE_STATE_LEVEL::ALL);
    ch->update_option();
    co_return ch;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> login<V>::assert_login(const game_reqs::login<V>& request)
{
    auto world = request.transfer.has_value() ? request.transfer->world : fb::config<uint32_t>("world");
    if (world == 0)
        world = fb::config<uint32_t>("world");
    auto&& resp = co_await this->server.http.post(
        "internal",
        "/in-game/login",
        internal_reqs::Login{world, request.id, request.name, fb::config<uint8_t>("id"), false, fb::process_role()});
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

template <fb::protocol::CLIENT_VERSION V>
std::string login<V>::elapsed_message(std::string_view dt)
{
    auto elapsed = this->server.now() - fb::model::datetime(dt);
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

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> login<V>::handle(fb::socket<character>& session, game_reqs::login<V>& request)
{
    session.encryption(request.enc_type, request.enc_key);
    fb::logger::info("{} has connected.", request.name);

    bool exists = this->server.characters.find(request.id) != nullptr;
    if (exists)
    {
        fb::logger::fatal(
            "Character {} already exists in server during initial insert - disconnecting duplicate session",
            request.name);
        co_return false;
    }

    auto delay = fb::config<uint32_t>("delay");
    co_await this->server.sleep(std::chrono::seconds(delay));

    if (co_await this->assert_login(request) == false)
        co_return false;

    auto ch = co_await this->init(request, session);
    if (ch == nullptr)
        co_return false;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
    log_data["level"]          = ch->level();
    if (auto map = ch->map(); map != nullptr)
    {
        log_data["map"]        = map->model().id;
        log_data["position_x"] = ch->position().x;
        log_data["position_y"] = ch->position().y;
    }
    this->server.log.write("login", log_data);

    co_return true;
}

template class login<fb::protocol::CLIENT_VERSION::v550>;
template class login<fb::protocol::CLIENT_VERSION::v565>;
template class login<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
