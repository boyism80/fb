#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/thread_params.h>
#include <fb/context.h>
#include <fb/model/model.h>
#include <stdexcept>
#include <fb/encoding.h>
#include <fb/config.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <json/writer.h>
#include <sstream>
#include <chrono>
#include <format>
#include <macro.h>
#include <unordered_map>

using namespace fb::game;
using namespace fb::model;
namespace internal_reqs = fb::protocol::internal::request;

character::character(fb::game::server& server, const initial_params& params) :
    stat(*this),
    storage_box(*this),
    marketplace(*this),
    matchmaker(*this),
    life(server,
         table::life[0],
         stat,
         fb::game::life::initial_params{
             {
              .id        = params.id,
              .position  = params.position,
              .direction = params.direction,
              }
}),
    listener(server.listener), id(params.id), _socket(params.socket), _pw(params.pw),
    _created_date(params.created_date), _updated_date(params.updated_date), _first_login_date(params.first_login_date),
    _name(params.name), _role(params.role), _birthday(params.birthday), _look(params.look), _color(params.color),
    _armor_color(params.armor_color), _weapon_color(params.weapon_color), _shield_color(params.shield_color),
    _experience(params.exp), _gender(params.gender), _state(params.state), _level(params.level),
    _class(params.class_type), _promotion(params.promotion), _money(params.money), _mimicry(params.mimicry),
    _title(params.title), _nation(params.nation), _creature(params.creature), _last_afk_time(server.now()),
    _marriage(server.now()), _super_hide(params.super_hide)
{
    this->_ping_state.last_ping_time = server.now() - std::chrono::seconds(10);
}

character::~character()
{
    if (this->dialog != nullptr)
        this->dialog->release();
}

void character::on_init()
{
    life::on_init();
    this->items.owner(this->shared_from_this_as<character>());
    this->trade.owner(this->shared_from_this_as<character>());
    this->quests.owner(this->shared_from_this_as<character>());
}

async::task<size_t> character::send(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();

    auto socket_ptr = this->_socket.lock();
    if (socket_ptr == nullptr || !socket_ptr->is_open())
    {
        co_return 0;
    }

    auto wire = fb::stream(stream);
    if (socket_ptr->prepare_outbound(wire, encrypt, wrap) == false)
        throw std::runtime_error("unknown exception while send bytes");

    const auto queued = wire.size();

    auto frame = execution_context::current();
    if (frame == nullptr)
        co_return co_await this->send_immediate(stream, encrypt, wrap);

    auto* ctx = frame->slot<context>(context::local::slot_id());
    if (ctx == nullptr)
        co_return co_await this->send_immediate(stream, encrypt, wrap);

    const auto endpoint =
        std::shared_ptr<boost::asio::ip::tcp::socket>(socket_ptr,
                                                      static_cast<boost::asio::ip::tcp::socket*>(socket_ptr.get()));
    ctx->outbound.append(endpoint, std::move(wire));
    co_return queued;
}

async::task<size_t> character::send(const fb::protocol::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();

    auto socket_ptr = this->_socket.lock();
    if (socket_ptr == nullptr || !socket_ptr->is_open())
    {
        co_return 0;
    }

    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    co_await response.serialize(writer);
    co_return co_await this->send(stream, encrypt, wrap);
}

async::task<size_t> character::send_immediate(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();

    auto socket_ptr = this->_socket.lock();
    if (socket_ptr == nullptr || !socket_ptr->is_open())
    {
        co_return 0;
    }

    co_return co_await socket_ptr->send(stream, encrypt, wrap);
}

async::task<size_t> character::send_immediate(const fb::protocol::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();

    auto socket_ptr = this->_socket.lock();
    if (socket_ptr == nullptr || !socket_ptr->is_open())
    {
        co_return 0;
    }

    co_return co_await socket_ptr->send(response, encrypt, wrap);
}

OBJECT_TYPE character::what() const
{
    this->assert_thread();

    return OBJECT_TYPE::CHARACTER;
}

async::task<bool> character::map(std::shared_ptr<fb::game::map>      map,
                                 std::optional<fb::model::point16_t> position,
                                 map_options                         options)
{
    if (this->_thread == nullptr)
        co_return true;

    this->assert_thread();

    auto old_map      = this->_map;
    auto old_position = this->_position;

    if (this->_map != map)
    {
        if (this->trade.trading())
            this->trade.cancel();
    }

    auto switch_process = (map != nullptr && map->active == false);
    auto new_map_id     = map != nullptr ? std::make_optional(map->model.id) : std::optional<uint32_t>();
    auto new_position   = fb::model::point16_t();
    if (position.has_value())
        new_position = position.value();
    else if (map != nullptr)
        new_position = map->model.spawn_position().value_or(fb::model::point16_t{0, 0});
    else
        new_position = fb::model::point16_t{0, 0};

    auto callback    = std::move(options.callback);
    options.callback = {};

    if (switch_process)
    {
        if (this->map() == nullptr)
            co_return false;

        try
        {
            auto   world = fb::config<uint32_t>("world");
            auto&& resp  = co_await this->server.http.post(
                "internal",
                "/in-game/transfer",
                internal_reqs::Transfer{world, internal::Service::Game, map->model.host, this->name(), false});

            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
                break;

            case ERROR_CODE::SERVER_NOT_READY:
                throw std::runtime_error(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

            case ERROR_CODE::BANNED:
                throw std::runtime_error(
                    character::container::build_ban_message(resp.ban_reason, resp.ban_expire_date));

            default:
                throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), resp.error));
            }

            if (callback)
            {
                if (co_await callback() == false)
                    co_return false;
            }

            std::ignore = co_await this->map(nullptr);
            co_await this->server.save(*this);

            this->listener.on_transfer(*this, *map, new_position, resp.ip, resp.port);
        }
        catch (std::exception& e)
        {
            this->update_map();
            this->update_external(true);
            this->listener.on_message(*this, e.what(), MESSAGE_TYPE::STATE);
            co_return false;
        }
        catch (boost::system::error_code& /*e*/)
        {
            this->update_map();
            this->update_external(true);
            this->listener.on_message(*this, _TEXT(MESSAGE_NOT_READY_GAME_SERVER), MESSAGE_TYPE::STATE);
            co_return false;
        }

        if (old_map != map)
        {
            // Log map transfer event
            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(this->id);
            log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
            log_data["level"]          = this->level();
            if (old_map != nullptr)
            {
                log_data["old_map"]        = old_map->model.id;
                log_data["old_position_x"] = old_position.x;
                log_data["old_position_y"] = old_position.y;
            }
            if (new_map_id.has_value())
            {
                log_data["new_map"]        = new_map_id.value();
                log_data["new_position_x"] = new_position.x;
                log_data["new_position_y"] = new_position.y;
            }
            this->server.log.write("map_transfer", log_data);
        }
        co_return true;
    }

    if (co_await object::map(map, position, std::move(options)) == false)
        co_return false;

    if (callback)
    {
        if (co_await callback() == false)
            co_return false;
    }

    if (old_map != map)
    {
        // Log map transfer event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(this->id);
        log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
        log_data["level"]          = this->level();
        if (old_map != nullptr)
        {
            log_data["old_map"]        = old_map->model.id;
            log_data["old_position_x"] = old_position.x;
            log_data["old_position_y"] = old_position.y;
        }
        if (new_map_id.has_value())
        {
            log_data["new_map"]        = new_map_id.value();
            log_data["new_position_x"] = new_position.x;
            log_data["new_position_y"] = new_position.y;
        }
        this->server.log.write("map_transfer", log_data);
    }
    co_return true;
}

uint32_t character::limited_exp(uint32_t exp) const
{
    if (this->max_level())
        return exp;

    auto range         = table::ability[this->_class][this->_level].exp;
    auto multiplier    = this->server.exp_multiplier();
    auto limit_percent = 3.3 * multiplier;
    auto limited       = static_cast<uint32_t>(range / 100.0 * limit_percent + 1);
    return std::min(limited, exp);
}

uint32_t character::normal_attack_damage(MOB_SIZE size) const
{
    this->assert_thread();

    auto weapon = this->items.weapon();
    if (weapon == nullptr)
        return 1 + std::rand() % 5;

    auto& model = weapon->based<fb::model::weapon>();
    auto& range = size == MOB_SIZE::SMALL ? model.damage_small : model.damage_large;
    return std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
}

bool character::inited() const
{
    if (this->_thread == nullptr)
        return false;

    return true;
}

ROLE character::role() const
{
    return this->_role;
}

void character::role(ROLE value)
{
    this->assert_thread();

    if (this->_role == value)
        return;

    auto old    = this->_role;
    this->_role = value;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_role"]       = static_cast<int>(old);
    log_data["new_role"]       = static_cast<int>(value);
    this->server.log.write("role_change", log_data);
}

async::task<void> character::attack(DURATION duration)
{
    this->assert_thread();
    try
    {
        this->assert_state({STATE::RIDING, STATE::GHOST});
        co_await life::attack(duration);
    }
    catch (const std::exception& e)
    {
        this->message(e.what());
    }
}

void character::action(ACTION action, DURATION duration, uint8_t sound)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::RIDING});
        life::action(action, duration, sound);
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

const std::string& character::name() const
{
    this->assert_thread();

    return this->_name;
}

const std::optional<uint32_t>& fb::game::character::birthday() const
{
    return this->_birthday;
}

void fb::game::character::birthday(const std::optional<uint32_t>& value)
{
    this->assert_thread();

    if (this->_birthday == value)
        return;

    auto old        = this->_birthday;
    this->_birthday = value;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_birthday"]   = old.has_value() ? static_cast<Json::Int64>(old.value()) : Json::Value::null;
    log_data["new_birthday"]   = value.has_value() ? static_cast<Json::Int64>(value.value()) : Json::Value::null;
    this->server.log.write("birthday_change", log_data);
}

uint16_t character::look() const
{
    this->assert_thread();

    return this->_look;
}

void character::look(uint16_t value)
{
    this->assert_thread();

    if (this->_look == value)
        return;

    auto old_look = this->_look;
    this->_look   = value;
    this->update_external(true);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_look"]       = old_look;
    log_data["new_look"]       = value;
    this->server.log.write("look_change", log_data);
}

uint8_t character::color() const
{
    this->assert_thread();

    return this->_color;
}

void character::color(uint8_t value)
{
    this->assert_thread();

    this->_color = value;
    this->update_external(true);
}

std::optional<uint8_t> character::armor_color() const
{
    this->assert_thread();

    return this->_armor_color;
}

void character::armor_color(std::optional<uint8_t> value)
{
    this->assert_thread();

    this->_armor_color = value;
    this->update_external(true);
}

std::optional<uint8_t> character::weapon_color() const
{
    this->assert_thread();

    return this->_weapon_color;
}

void character::weapon_color(std::optional<uint8_t> value)
{
    this->assert_thread();

    this->_weapon_color = value;
    this->update_external(true);
}

std::optional<uint8_t> character::shield_color() const
{
    this->assert_thread();

    return this->_shield_color;
}

void character::shield_color(std::optional<uint8_t> value)
{
    this->assert_thread();

    this->_shield_color = value;
    this->update_external(true);
}

const std::optional<character_appearance>& character::mimicry() const
{
    this->assert_thread();
    return this->_mimicry;
}

void character::mimicry(std::optional<character_appearance> value)
{
    this->assert_thread();

    this->_mimicry = std::move(value);
    this->update_external(true);
}

NATION character::nation() const
{
    this->assert_thread();

    return this->_nation;
}

bool character::nation(NATION value)
{
    this->assert_thread();

    if (value != NATION::GOGURYEO && value != NATION::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

CREATURE character::creature() const
{
    this->assert_thread();

    return this->_creature;
}

bool character::creature(CREATURE value)
{
    static const std::unordered_set<CREATURE> valid_creatures = {CREATURE::DRAGON,
                                                                 CREATURE::PHOENIX,
                                                                 CREATURE::TIGER,
                                                                 CREATURE::TURTLE};
    this->assert_thread();

    if (valid_creatures.contains(value) == false)
        return false;

    this->_creature = value;
    return true;
}

uint8_t character::level() const
{
    this->assert_thread();

    return this->_level;
}

void character::level(uint8_t value)
{
    this->assert_thread();

    if (this->_level == value)
        return;

    auto old_level = this->_level;
    this->_level   = value;
    this->update(UPDATE_STATE_LEVEL::ALL);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_level"]      = old_level;
    log_data["new_level"]      = value;
    this->server.log.write("level_change", log_data);
}

bool character::level_up()
{
    this->assert_thread();

    if (this->max_level())
        return false;

    auto& ability = table::ability[this->_class][this->_level];
    {
        auto batch = this->batch_update();
        this->stat.base_str(this->stat.base_str() + ability.strength);
        this->stat.base_int(this->stat.base_int() + ability.intelligence);
        this->stat.base_dex(this->stat.base_dex() + ability.dexterity);
        this->stat.base_hp(this->stat.base_hp() + ability.hp + std::rand() % 10);
        this->stat.base_mp(this->stat.base_mp() + ability.mp + std::rand() % 10);
        this->stat.hp(this->stat.base_hp());
        this->stat.mp(this->stat.base_mp());
    }

    auto old_level = this->_level;
    this->level(this->_level + 1);
    this->message(_TEXT(MESSAGE_LEVEL_UP));

    // Log level up event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_level"]      = old_level;
    log_data["new_level"]      = this->_level;
    log_data["base_hp"]        = static_cast<Json::Int64>(this->stat.base_hp());
    log_data["base_mp"]        = static_cast<Json::Int64>(this->stat.base_mp());
    this->server.log.write("level_up", log_data);

    this->listener.on_level_up(*this);
    return true;
}

bool character::max_level() const
{
    this->assert_thread();

    return table::ability[this->_class].contains(this->_level + 1) == false;
}

GENDER character::gender() const
{
    this->assert_thread();

    return this->_gender;
}

void character::gender(GENDER value)
{
    this->assert_thread();

    if (this->_gender == value)
        return;

    auto old_gender = this->_gender;
    this->_gender   = value;
    this->update_external(true);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_gender"]     = static_cast<int>(old_gender);
    log_data["new_gender"]     = static_cast<int>(value);
    this->server.log.write("gender_change", log_data);
}

STATE character::state() const
{
    this->assert_thread();

    if (this->_mimicry.has_value() && this->_mimicry->disguise.has_value())
        return STATE::DISGUISE;
    return this->_state;
}

STATE character::state_to(const fb::game::object& to, STATE state) const
{
    this->assert_thread();

    if (to.is(OBJECT_TYPE::CHARACTER) == false)
        return state;

    if (state != STATE::CLOACK && state != STATE::ADV_CLOACK)
        return state;

    const auto& ch = static_cast<const fb::game::character&>(to);
    if (this == &ch)
        return STATE::HALF_CLOACK;

    if (ch.detect())
        return STATE::HALF_CLOACK;

    auto& g1 = this->_group_id;
    auto& g2 = ch._group_id;
    if (g1 != std::nullopt && g2 != std::nullopt && g1.value() == g2.value())
        return STATE::HALF_CLOACK;

    if (ch.role() > ROLE::USER && this->role() <= ch.role())
        return STATE::HALF_CLOACK;

    return state;
}

void character::state(STATE value)
{
    this->assert_thread();

    if (value == STATE::DISGUISE)
        return;

    auto old_state = this->_state;
    this->_state   = value;

    this->update_external(true);

    // Log revive event (state change from GHOST to NORMAL)
    if (old_state == STATE::GHOST && value == STATE::NORMAL)
    {
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(this->id);
        log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
        log_data["level"]          = this->level();
        auto map                   = this->map();
        if (map != nullptr)
        {
            log_data["map"]        = map->model.id;
            log_data["position_x"] = this->position().x;
            log_data["position_y"] = this->position().y;
        }
        this->server.log.write("revive", log_data);
    }
}

CLASS character::cls() const
{
    this->assert_thread();

    return this->_class;
}

void character::cls(CLASS value)
{
    this->assert_thread();

    if (this->_class == value)
        return;

    auto old_class = this->_class;
    this->_class   = value;
    this->update_id();

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_class"]      = static_cast<int>(old_class);
    log_data["new_class"]      = static_cast<int>(value);
    this->server.log.write("class_change", log_data);
}

uint8_t character::promotion() const
{
    this->assert_thread();
    return this->_promotion;
}

void character::promotion(uint8_t value)
{
    this->assert_thread();

    if (this->_promotion == value)
        return;

    auto old_promotion = this->_promotion;
    this->_promotion   = value;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_promotion"]  = old_promotion;
    log_data["new_promotion"]  = value;
    this->server.log.write("promotion_change", log_data);
}

uint32_t character::exp() const
{
    this->assert_thread();

    return this->_experience;
}

void character::exp(uint32_t value)
{
    this->assert_thread();

    if (this->_experience == value)
        return;

    this->_experience = value;
    this->update(UPDATE_STATE_LEVEL::EXP_MONEY);
}

uint32_t character::add_exp(uint32_t value, bool limit, bool notify)
{
    this->assert_thread();

    // Apply experience multiplier
    auto multiplier = this->server.exp_multiplier();
    value           = static_cast<uint32_t>(value * multiplier);

    if (limit)
        value = this->limited_exp(value);

    auto capacity = 0xFFFFFFFF - this->_experience;
    auto lack     = 0;

    try
    {
        // When class is NONE, cap exp to exactly what is needed for level 5
        if (this->_class == CLASS::NONE)
        {
            auto require = table::ability.stacked_exp(CLASS::NONE, 5);
            if (this->_experience > require)
                value = 0;

            if (this->_experience + value > require)
                value = require - this->_experience;
        }

        if (value > 0)
        {
            if (value > capacity)
            {
                lack = value - capacity;
                this->exp(this->_experience + capacity);
            }
            else
            {
                this->exp(this->_experience + value);
            }

            if (notify)
                this->message(std::format(_TEXT(MESSAGE_EXP_GAINED), value, int(this->experience_percent())));
        }

        if (table::ability.contains(this->_class) == false)
            throw std::runtime_error("what?");

        while (true)
        {
            if (this->max_level())
                break;

            auto& next = table::ability[this->_class][this->_level];
            if (next.exp == 0)
                break;

            if (this->_experience < table::ability.stacked_exp(this->_class, this->_level))
                break;

            if (this->level_up() == false)
                break;
        }

        if (this->_class == CLASS::NONE && this->max_level())
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_REQUIRE_CLASS));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }

    return lack;
}

uint32_t character::reduce_exp(uint32_t value)
{
    this->assert_thread();

    if (this->_experience < value)
    {
        uint32_t lack     = value - this->_experience;
        this->_experience = 0;
        this->update(UPDATE_STATE_LEVEL::EXP_MONEY);
        return lack;
    }
    else
    {
        this->_experience -= value;
        this->update(UPDATE_STATE_LEVEL::EXP_MONEY);
        return 0;
    }
}

uint32_t character::experience_remained() const
{
    this->assert_thread();

    if (this->max_level())
        return 0;

    if (table::ability.contains(this->_class) == false)
        return 0;

    if (table::ability[this->_class].contains(this->_level) == false)
        return 0;

    return table::ability.stacked_exp(this->_class, this->_level) - this->exp();
}

float character::experience_percent() const
{
    this->assert_thread();

    if (this->max_level())
        return std::min(100.0f, (this->_experience / float(0xFFFFFFFF)) * 100.0f);

    auto level          = this->level();
    auto required       = table::ability[this->_class][level].exp;
    auto prev_stack_exp = uint32_t{0};
    if (table::ability[this->_class].contains(level - 1))
        prev_stack_exp = table::ability.stacked_exp(this->_class, level - 1);
    else if (table::ability[CLASS::NONE].contains(level - 1))
        prev_stack_exp = table::ability.stacked_exp(CLASS::NONE, level - 1);

    return std::min(100.0f, ((this->_experience - prev_stack_exp) / float(required)) * 100.0f);
}

uint32_t character::money() const
{
    this->assert_thread();

    return this->_money;
}

void character::money(uint32_t value)
{
    this->assert_thread();

    if (this->_money == value)
        return;

    auto old_money = this->_money;
    this->_money   = value;
    this->update(UPDATE_STATE_LEVEL::EXP_MONEY);

    // Log money changed event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["old_money"]      = static_cast<Json::Int64>(old_money);
    log_data["new_money"]      = static_cast<Json::Int64>(value);
    log_data["amount"] = static_cast<Json::Int64>(static_cast<int64_t>(value) - static_cast<int64_t>(old_money));
    this->server.log.write("money_changed", log_data);
}

uint32_t character::money_add(uint32_t value) // Returns remaining value that could not be added
{
    this->assert_thread();

    uint32_t capacity = 0xFFFFFFFF - this->_money;
    uint32_t lack     = 0;
    if (value > capacity)
    {
        this->money(this->_money + capacity);
        lack = value - capacity;
    }
    else
    {
        this->money(this->_money + value);
    }

    return lack;
}

void character::money_reduce(uint32_t value)
{
    this->assert_thread();
    value = std::min(this->_money, value);
    this->money(this->_money - value);
}

fb::game::cash* character::money_drop(uint32_t value)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::RIDING, STATE::GHOST});

        if (value == 0)
            return 0;

        value = std::min(this->_money, value);
        this->money_reduce(value);

        // TODO: Phase 3 - Convert to smart pointer return type
        // For now, use make_shared but return raw pointer for compatibility
        auto cash_shared = this->server.make<fb::game::cash>(value);
        auto cash        = cash_shared.get();
        cash->map(this->_map, this->_position);
        this->action(ACTION::PICKUP, DURATION::PICKUP);
        this->message(_TEXT(MESSAGE_MONEY_DROP));
        return cash;
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }

    return 0;
}

bool character::option(OPTION key) const
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    return this->_options[opt];
}

void character::option(OPTION key, bool value, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        return;

    if (this->_options[opt] == value)
        return;

    this->update(UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL);
    this->_options[opt] = value;
    this->update_option();

    if (notify)
        this->listener.on_option_changed(*this, key, value);
}

bool character::option_toggle(OPTION key, bool notify)
{
    this->assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::EFFECT_SOUND))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    this->option(key, !this->_options[opt], notify);
    return this->_options[opt];
}

void character::update_option()
{
    this->listener.on_update_option(*this);
}

void character::update_map(const fb::game::map& map)
{
    this->listener.on_update_map(*this, map);
}

void character::update_map()
{
    if (this->_map != nullptr)
        this->update_map(*this->_map);
}

void character::update_map(const fb::game::map&        map,
                           const fb::model::point16_t& begin,
                           const fb::model::size8_t&   size,
                           uint16_t                    crc)
{
    this->listener.on_update_map(*this, map, begin, size, crc);
}

void character::update_bgm(uint16_t bgm, uint8_t volume)
{
    this->listener.on_update_bgm(*this, bgm, volume);
}

void character::update_buff()
{
    this->listener.on_update_buff(*this, this->buffs);
}

void character::update_internal()
{
    this->listener.on_update_internal(*this);
}

void character::update_time(uint16_t hours)
{
    this->listener.on_update_time(*this, hours);
}

void character::init()
{
    this->listener.on_character_init(*this);
}

void character::update_position()
{
    this->listener.on_update_position(*this);
}

void character::screen_refresh()
{
    this->listener.on_screen_refresh(*this);
}

const std::string& character::title() const
{
    this->assert_thread();

    return this->_title;
}

void character::title(std::string_view value)
{
    this->assert_thread();

    this->_title = std::string(value);
}

const std::optional<uint32_t>& character::group_id() const
{
    this->assert_thread();

    return this->_group_id;
}

std::optional<uint32_t>& character::group_id()
{
    this->assert_thread();

    return this->_group_id;
}

void character::group_id(uint32_t gid)
{
    this->assert_thread();

    this->_group_id = gid;
}

void character::group_reset()
{
    this->assert_thread();

    this->_group_id.reset();
}

const std::optional<uint32_t>& character::clan_id() const
{
    this->assert_thread();
    return this->_clan_id;
}

void character::clan_id(std::optional<uint32_t> value)
{
    this->assert_thread();

    this->_clan_id = value;
}

void character::clan_reset()
{
    this->assert_thread();

    this->_clan_id.reset();
    this->update_external(false);
}

void character::assert_state(STATE value) const
{
    this->assert_thread();

    static const auto error = std::map<STATE, const std::string>{
        {STATE::GHOST,    _TEXT(MESSAGE_EXCEPTION_GHOST)   },
        {STATE::RIDING,   _TEXT(MESSAGE_EXCEPTION_RIDDING) },
        {STATE::DISGUISE, _TEXT(MESSAGE_EXCEPTION_DISGUISE)}
    };

    if (this->state() == value)
        throw std::runtime_error(error.at(value));
}

void character::assert_state(const std::vector<STATE>& values) const
{
    this->assert_thread();

    for (auto value : values)
    {
        this->assert_state(value);
    }
}

bool character::move(const fb::model::point16_t& before)
{
    this->assert_thread();

    return this->move(this->_direction, before);
}

bool character::move(DIRECTION direction, const fb::model::point16_t& before)
{
    this->assert_thread();

    if (this->_position != before)
    {
        this->update_position();
        return false;
    }
    else if (object::move(direction) == false)
    {
        this->update_position();
        return false;
    }
    else
    {
        return true;
    }
}

void character::ride(mob& horse)
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        if (this->state() == STATE::RIDING)
            throw std::runtime_error(_TEXT(MESSAGE_RIDE_ALREADY_RIDE));

        if (horse.based<fb::model::mob>() != table::mob[fb::model::const_value::mob::horse])
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_NO_CONVEYANCE));

        if (horse.map() != this->_map)
            throw std::runtime_error(_TEXT(MESSAGE_ERROR_UNKNOWN));

        std::ignore = horse.map(nullptr);
        this->state(STATE::RIDING);
        horse.kill();
        this->message(_TEXT(MESSAGE_RIDE_ON));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

void character::ride()
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});

        auto front = this->forward(OBJECT_TYPE::MOB);
        if (front == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_NO_CONVEYANCE));

        this->ride(static_cast<mob&>(*front));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

void character::unride()
{
    this->assert_thread();

    try
    {
        this->assert_state({STATE::GHOST, STATE::DISGUISE});
        if (this->state() != STATE::RIDING)
            throw std::runtime_error(_TEXT(MESSAGE_RIDE_UNRIDE));

        auto& model = table::mob[fb::model::const_value::mob::horse];
        auto  horse = this->server.make<mob>(model, mob::initial_params{.alive = true});
        horse->map(this->_map, this->front_position());

        this->state(STATE::NORMAL);
        this->message(_TEXT(MESSAGE_RIDE_OFF));
    }
    catch (std::exception& e)
    {
        this->message(e.what());
    }
}

bool character::alive() const
{
    this->assert_thread();

    return this->_state != STATE::GHOST;
}

bool character::condition(const std::vector<fb::model::dsl>& conditions) const
{
    this->assert_thread();

    for (auto& dsl : conditions)
    {
        switch (dsl.header)
        {
        case DSL::level:
        {
            auto params = fb::model::dsl::level(dsl.params);
            if (params.min.has_value() && *params.min > this->_level)
                return false;

            if (params.max.has_value() && *params.max < this->_level)
                return false;
        }
        break;

        case DSL::gender:
        {
            auto params = fb::model::dsl::gender(dsl.params);
            if (ENUM_IN(params.value, this->_gender) == false)
                return false;
        }
        break;

        case DSL::strength:
        {
            auto params = fb::model::dsl::strength(dsl.params);
            if (params.value > this->stat.str())
                return false;
        }
        break;

        case DSL::intelligence:
        {
            auto params = fb::model::dsl::intelligence(dsl.params);
            if (params.value > this->stat.intelligence())
                return false;
        }
        break;

        case DSL::dexterity:
        {
            auto params = fb::model::dsl::dexterity(dsl.params);
            if (params.value > this->stat.dex())
                return false;
        }
        break;

        case DSL::promotion:
        {
            auto params = fb::model::dsl::promotion(dsl.params);
            if (params.value > this->_promotion)
                return false;
        }
        break;

        case DSL::class_t:
        {
            auto params = fb::model::dsl::class_t(dsl.params);
            if (ENUM_IN(params.value, this->_class) == false)
                return false;
        }

        case DSL::role:
        {
            auto params = fb::model::dsl::role(dsl.params);
            if (params.value > this->_role)
                return false;
        }
        break;
        }
    }

    return true;
}

void character::message(std::string_view message, MESSAGE_TYPE type)
{
    this->assert_thread();
    this->listener.on_message(*this, message, type);
}

async::task<void> character::whisper(std::string receiver_name, std::string message)
{
    if (this->option(OPTION::WHISPER) == false)
        throw std::runtime_error(_TEXT(MESSAGE_WHISPER_DISABLED_MINE));

    auto sender_weak = this->weak_from_this_as<character>();
    auto sender_name = this->name();

    auto current_thread = this->server.threads.current();
    if (current_thread != nullptr)
    {
        auto params   = current_thread->template data<thread_params>();
        auto receiver = params->characters.find(receiver_name);
        if (receiver != nullptr)
        {
            if (receiver->option(OPTION::WHISPER) == false)
                throw std::runtime_error(std::format(_TEXT(MESSAGE_WHISPER_DISABLED_TARGET), receiver_name));

            auto target_name = receiver->name();
            receiver->message(std::format("{}\" {}", sender_name, message), MESSAGE_TYPE::NOTIFY);
            this->message(std::format("{}< {}", target_name, message), MESSAGE_TYPE::NOTIFY);

            auto log_data             = Json::Value();
            log_data["sender_id"]     = static_cast<Json::Int64>(this->id);
            log_data["sender_name"]   = UTF8(sender_name, PLATFORM::WINDOWS);
            log_data["receiver_id"]   = static_cast<Json::Int64>(receiver->id);
            log_data["receiver_name"] = UTF8(target_name, PLATFORM::WINDOWS);
            log_data["message"]       = UTF8(message, PLATFORM::WINDOWS);
            this->server.log.write("whisper", log_data);
            co_return;
        }
    }

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/in-game/whisper",
                                                  internal_reqs::Whisper{world, sender_name, receiver_name, message});
    co_await this->server.threads.switching(sender_weak);

    character::container::assert_whisper(resp.error, resp.to);

    std::weak_ptr<character> receiver_weak;
    {
        auto guard    = co_await this->server.characters.enter_read_async();
        auto receiver = guard.value().find(resp.to);
        if (receiver == nullptr)
            co_return;

        receiver_weak = receiver->weak_from_this_as<character>();
    }

    auto before = this->server.threads.current();
    co_await this->server.threads.switching(receiver_weak);

    auto receiver = receiver_weak.lock();
    if (receiver != nullptr)
    {
        receiver->message(std::format("{}> {}", resp.from, resp.message), MESSAGE_TYPE::NOTIFY);

        auto recv_log             = Json::Value();
        recv_log["sender_name"]   = UTF8(resp.from, PLATFORM::WINDOWS);
        recv_log["receiver_id"]   = static_cast<Json::Int64>(receiver->id);
        recv_log["receiver_name"] = UTF8(resp.to, PLATFORM::WINDOWS);
        recv_log["message"]       = UTF8(resp.message, PLATFORM::WINDOWS);
        this->server.log.write("whisper", recv_log);
    }

    if (before != nullptr)
        co_await before->switching();

    this->message(std::format("{}< {}", receiver_name, message), MESSAGE_TYPE::NOTIFY);

    auto log_data             = Json::Value();
    log_data["sender_id"]     = static_cast<Json::Int64>(this->id);
    log_data["sender_name"]   = UTF8(sender_name, PLATFORM::WINDOWS);
    log_data["receiver_name"] = UTF8(receiver_name, PLATFORM::WINDOWS);
    log_data["message"]       = UTF8(message, PLATFORM::WINDOWS);
    this->server.log.write("whisper", log_data);
}

fb::thread* character::thread() const
{
    if (this->_thread != nullptr)
        return this->_thread;
    else
        return this->server.threads.modular(this->id);
}

void character::thread(fb::thread* value)
{
    this->assert_thread();
    this->_thread = value;
}

void character::assert_thread() const
{
    object::assert_thread();
}

void character::update(UPDATE_STATE_LEVEL value)
{
    this->assert_thread();

    if (this->_batch_mode)
    {
        this->_pending_update |= value;
        return;
    }

    this->listener.on_update(*this, value);
}

void character::kill(DESTROY_TYPE destroy_type)
{
    this->assert_thread();
    life::kill(destroy_type);

    this->death_penalty();
    this->state(STATE::GHOST);
}

void character::notify_death(std::shared_ptr<fb::game::object> killer)
{
    this->assert_thread();
    this->listener.on_dead(*this, killer);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->id);
    log_data["character_name"] = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["level"]          = this->level();
    auto map                   = this->map();
    if (map != nullptr)
    {
        log_data["map"]        = map->model.id;
        log_data["position_x"] = this->position().x;
        log_data["position_y"] = this->position().y;
    }
    if (killer != nullptr && killer->is(OBJECT_TYPE::CHARACTER))
    {
        auto& killer_ch         = static_cast<character&>(*killer);
        log_data["killer_id"]   = static_cast<Json::Int64>(killer_ch.id);
        log_data["killer_name"] = UTF8(killer_ch.name(), PLATFORM::WINDOWS);
    }
    this->server.log.write("death", log_data);
}

async::task<void> character::settle_kills(mob_vector dead)
{
    this->assert_thread();

    auto groups = std::unordered_map<uint32_t, mob_vector>{};
    for (auto& m : dead)
    {
        if (m == nullptr)
            continue;
        groups[m->based<fb::model::mob>().id].push_back(m);
    }

    auto self = this->shared_from_this_as<character>();
    for (auto& [id, mobs] : groups)
    {
        auto path = std::format("scripts/mob/{}.lua", id);
        auto func = std::format("ON_MOB_KILL_{}", id);
        auto lua  = this->server.lua.new_ctx_guard(path, func);
        if (lua)
        {
            lua->pushobject(*self);
            lua->new_table();
            for (auto i = 0; i < static_cast<int>(mobs.size()); i++)
            {
                lua->pushobject(*mobs[i]);
                lua->rawseti(-2, i + 1);
            }

            try
            {
                std::ignore = co_await lua->call(2);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("error in ON_MOB_KILL_{}: {}", id, e.what());
            }
            catch (...)
            {
                fb::logger::fatal("unknown error in ON_MOB_KILL_{}", id);
            }
        }

        for (auto& m : mobs)
        {
            auto owner = m->owner.lock();
            if (owner != nullptr)
            {
                owner->detach_spawned_mob(*m);
                m->kill(DESTROY_TYPE::DEAD);
                continue;
            }

            this->listener.on_dead(*m, self);
            co_await m->drop_items();
            this->award_exp(*m);
            m->kill(DESTROY_TYPE::DEAD);
        }
    }
}

void character::award_exp(const fb::game::mob& mob)
{
    this->assert_thread();

    auto& group_id = this->group_id();
    auto  map      = this->map();
    auto  exp      = mob.based<fb::model::mob>().exp;

    if (group_id.has_value() && map != nullptr)
    {
        auto  guard      = this->server.groups.enter_read(group_id.value());
        auto& group      = guard.value();
        auto  nears      = group->nears(*map, this->position());
        auto  size       = nears.size();
        auto  divide_exp = exp / size;
        for (auto& member : nears)
        {
            auto shared_ptr = member.lock();
            if (shared_ptr == nullptr)
                continue;

            shared_ptr->add_exp(divide_exp, true, true);
        }
    }
    else
    {
        this->add_exp(exp, true, true);
    }
}

async::task<void> character::damage_to(const damage_list& targets, const damage_opts& opts)
{
    this->assert_thread();
    auto dead = this->damage_targets(targets, opts);
    if (dead.empty() == false)
        co_await this->settle_kills(std::move(dead));
    co_return;
}

fb::protocol::internal::Character character::to_protocol() const
{
    this->assert_thread();

    if (!this->_first_login_date.has_value())
        this->_first_login_date = this->server.now();

    auto dto             = fb::protocol::internal::Character();
    dto.id               = this->id;
    dto.name             = this->_name;
    dto.pw               = this->_pw;
    dto.birth            = this->_birthday;
    dto.created_date     = this->_created_date.to_string();
    dto.updated_date     = this->server.now().to_string();
    dto.first_login_date = this->_first_login_date->to_string();
    dto.role             = static_cast<uint8_t>(this->_role);
    dto.look             = this->_look;
    dto.color            = this->_color;
    dto.gender           = static_cast<uint8_t>(this->_gender);
    dto.nation           = static_cast<uint8_t>(this->_nation);
    dto.creature         = static_cast<uint8_t>(this->_creature);
    dto.map              = this->_map != nullptr ? this->_map->model.id : 0;
    dto.position         = fb::protocol::internal::Position{this->_position.x, this->_position.y};
    dto.direction        = static_cast<uint8_t>(this->_direction);
    dto.state            = static_cast<uint8_t>(this->_state);
    dto.class_type       = static_cast<uint8_t>(this->_class);
    dto.promotion        = this->_promotion;
    dto.level            = this->_level;
    dto.exp              = this->_experience;
    dto.money            = this->_money;
    dto.deposited_money  = this->items.deposited();
    if (this->_mimicry.has_value())
    {
        auto const& p         = this->_mimicry.value();
        auto        state_opt = p.state.has_value() ? std::optional<uint8_t>(static_cast<uint8_t>(p.state.value()))
                                                    : std::optional<uint8_t>();
        dto.mimicry           = fb::protocol::internal::Mimicry(static_cast<uint8_t>(p.gender),
                                                      state_opt,
                                                      p.hair,
                                                      p.hair_color,
                                                      p.weapon,
                                                      p.weapon_color,
                                                      p.armor,
                                                      p.armor_color,
                                                      p.shield,
                                                      p.shield_color,
                                                      p.disguise);
    }
    else
    {
        dto.mimicry = std::nullopt;
    }
    dto.hp               = this->stat.hp();
    dto.base_hp          = this->stat.base_hp();
    dto.additional_hp    = 0;
    dto.mp               = this->stat.mp();
    dto.base_mp          = this->stat.base_mp();
    dto.additional_mp    = 0;
    dto.weapon_color     = this->_weapon_color;
    dto.helmet_color     = std::nullopt;
    dto.armor_color      = this->_armor_color;
    dto.shield_color     = this->_shield_color;
    dto.ring_left_color  = std::nullopt;
    dto.ring_right_color = std::nullopt;
    dto.aux_top_color    = std::nullopt;
    dto.aux_bot_color    = std::nullopt;
    dto.title            = this->_title;
    dto.super_hide       = this->_super_hide;

    for (auto& [_, buff] : this->buffs)
    {
        auto remaining_ms = buff->remaining().total_milliseconds();
        if (remaining_ms < 0)
            remaining_ms = 0;

        // `internal::Buff.time` is seconds, not milliseconds.
        auto remaining_s = static_cast<uint32_t>(remaining_ms / 1000);
        dto.buffs.push_back({buff->model.id, remaining_s});
    }

    return dto;
}

const fb::game::marriage& character::marriage() const
{
    this->assert_thread();
    return this->_marriage;
}

void character::marriage(const fb::game::marriage& value)
{
    this->assert_thread();
    this->_marriage = value;
}

void character::browse_ch(const character& ch)
{
    this->listener.on_browse_character(*this, ch);
}

void character::item_tooltip(const item& item, uint16_t position)
{
    this->listener.on_item_tooltip(*this, item, position);
}

void character::show_user_list()
{
    this->listener.on_show_user_list(*this);
}

void character::show_world_map(uint32_t id, uint16_t index)
{
    this->listener.on_show_world_map(*this, id, index);
}

void character::timer(uint32_t time, TIMER_TYPE type)
{
    this->listener.on_timer(*this, time, type);
}
void character::weather(WEATHER_TYPE weather)
{
    this->listener.on_weather(*this, weather);
}

void character::ping(uint32_t token)
{
    this->assert_thread();
    this->listener.on_ping(*this, token);
}

void character::save_ack()
{
    this->assert_thread();
    this->listener.on_save(*this);
}

void character::bulk_objects_update(const std::vector<object*>& objects)
{
    this->assert_thread();
    this->listener.on_bulk_update(*this, objects);
}

void character::ad(uint32_t width, uint32_t height, std::string_view url, uint8_t time)
{
    this->assert_thread();
    this->listener.on_ad(*this, width, height, url, time);
}

void character::web(uint8_t type, std::string_view url, std::string_view message)
{
    this->assert_thread();
    this->listener.on_web(*this, type, url, message);
}

void character::ui(uint8_t screen)
{
    this->assert_thread();
    this->listener.on_ui(*this, screen);
}

void character::item_throw_confirm(uint8_t slot)
{
    this->assert_thread();
    this->listener.on_item_throw_confirm(*this, slot);
}

void character::freeze(bool value)
{
    this->assert_thread();
    this->listener.on_freeze(*this, value);
}

void character::friends_sync(uint8_t enabled)
{
    this->assert_thread();
    this->listener.on_friends_sync(*this, enabled);
}

void character::holyday_screen(uint8_t                          screen,
                               uint8_t                          hair,
                               fb::model::enum_value::DIRECTION direction,
                               const fb::model::point<uint8_t>& position)
{
    this->assert_thread();
    this->listener.on_holyday_screen(*this, screen, hair, direction, position);
}

void character::update_id()
{
    this->listener.on_update_id(*this);
}

void character::weapon_damage(uint16_t value)
{
    this->assert_thread();

    this->_weapon_damage = value;
}

uint16_t character::weapon_damage() const
{
    return this->_weapon_damage;
}

void character::detect(bool value)
{
    this->assert_thread();
    this->_detect = value;

    for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (ch->state() != STATE::CLOACK && ch->state() != STATE::ADV_CLOACK)
            continue;

        ch->update_external(*this, true);
    }
}

bool character::detect() const
{
    return this->_detect;
}

std::shared_ptr<fb::game::mob>
character::spawn_mob(const fb::model::mob& model, const fb::model::point16_t& position, bool owned, bool notify)
{
    this->assert_thread();
    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    auto  params    = fb::game::mob::initial_params{.alive = true, .owner = owned ? this : nullptr};
    auto& mob_model = static_cast<const fb::model::mob&>(model);
    auto  mob       = std::make_shared<fb::game::mob>(this->server, mob_model, params);
    mob->map(map, position, {.notify = notify});

    if (owned)
        this->_spawned_mobs.push_back(mob);

    return mob;
}

const std::vector<std::shared_ptr<fb::game::mob>>& character::spawned_mobs() const
{
    this->assert_thread();
    return this->_spawned_mobs;
}

bool character::detach_spawned_mob(fb::game::mob& mob)
{
    this->assert_thread();
    auto owner = mob.owner.lock();
    if (owner == nullptr)
        return false;

    if (owner.get() != this)
        return false;

    if (this->_spawned_mobs.size() == 0)
        return false;

    for (auto it = this->_spawned_mobs.begin(); it != this->_spawned_mobs.end(); ++it)
    {
        if (it->get() == &mob)
        {
            this->_spawned_mobs.erase(it);
            return true;
        }
    }
    return false;
}

void character::bright(uint8_t value)
{
    this->listener.on_bright(*this, value);
}

bool character::super_hide() const
{
    return this->_super_hide;
}

void character::super_hide(bool enabled)
{
    this->assert_thread();
    this->_super_hide = enabled;
    if (enabled)
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
        {
            if (this->hidden(*obj))
                this->hide(*obj);
            else
                this->update_external(*obj, false);
        }
    }
    else
    {
        for (auto& obj : this->nears(OBJECT_TYPE::CHARACTER))
        {
            this->update_external(*obj, false);
        }
    }
}

bool character::hidden(const fb::game::object& target) const
{
    if (this->super_hide() == false)
        return false;

    if (target.is(OBJECT_TYPE::CHARACTER) == false)
        return true;

    auto& ch = static_cast<const fb::game::character&>(target);
    return this->role() > ch.role();
}

bool character::hidden(ROLE role) const
{
    return this->role() > role;
}

void character::update_last_afk_time()
{
    this->assert_thread();
    this->_last_afk_time = this->server.now();
}

fb::model::datetime& character::last_afk_time()
{
    this->assert_thread();
    return this->_last_afk_time;
}

async::task<void> character::death_penalty()
{
    this->assert_thread();
    auto buff_keys = std::vector<uint32_t>{};
    for (auto& [k, v] : this->buffs)
    {
        buff_keys.push_back(k);
    }
    for (auto k : buff_keys)
    {
        this->buffs.remove(k);
    }

    auto money = this->money();
    if (money > 0)
    {
        this->money_reduce(money);

        auto cash_shared = this->server.make<fb::game::cash>(money);
        auto cash        = cash_shared.get();
        cash->death_uid(this->id);
        std::ignore = co_await cash->map(this->map(), this->position());
    }

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = this->items[i];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT))
        {
            auto  equipment       = std::static_pointer_cast<fb::game::equipment>(item);
            auto& equipment_model = equipment->based<fb::model::equipment>();
            auto  penalty         = equipment_model.durability * fb::model::const_value::death_penalty::durability;
            if (equipment->durability_down(penalty))
            {
                this->items.remove(i, 1, ITEM_DELETE_TYPE::DESTROY);
                continue;
            }
        }

        if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
        {
            this->items.drop(i, item->count(), false, ITEM_DELETE_TYPE::NONE);
            item->container(nullptr);
            item->death_uid(this->id);
            std::ignore = co_await item->map(this->map(), this->position());
        }
    }

    for (auto& [parts, equipment] : this->items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model   = equipment->based<fb::model::equipment>();
        auto  penalty = model.durability * fb::model::const_value::death_penalty::durability;
        if (equipment->durability_down(penalty))
        {
            this->items.equipment_off(parts);
            this->message(std::format(_TEXT(MESSAGE_EQUIPMENT_BROKEN), equipment->name()));
            equipment.reset();
            continue;
        }

        if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
        {
            this->items.equipment_off(parts);
            equipment->container(nullptr);
            equipment->death_uid(this->id);
            std::ignore = co_await equipment->map(this->map(), this->position());
        }
        else if (this->items.free())
        {
            this->items.equipment_off(parts);
            this->items.add(equipment);
        }
    }

    auto cls   = this->cls();
    auto level = this->level();
    if (table::ability.contains(cls) && table::ability[cls].contains(level) && table::ability[cls].contains(level - 1))
    {
        auto penalty = uint32_t(table::ability[cls][level].exp * fb::model::const_value::death_penalty::exp);
        auto gained  = this->exp() - table::ability.stacked_exp(cls, level - 1);

        penalty = std::min(gained, penalty);
        if (penalty > 0)
        {
            this->exp(this->exp() - penalty);
            this->message(std::format(_TEXT(MESSAGE_EXP_LOST), penalty));
        }
    }
}

bool character::reward(const std::vector<fb::model::dsl>& reward)
{
    this->assert_thread();
    if (this->items.is_rewardable(reward) == false)
        return false;

    auto money = 0;
    auto exp   = 0;
    for (auto& item : reward)
    {
        switch (item.header)
        {
        case fb::model::enum_value::DSL::item:
        {
            auto  params = fb::model::dsl::item(item.params);
            auto& model  = table::item[params.id];
            auto  item   = model.make(this->server, params.count);
            if (params.durability.has_value())
            {
                if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT))
                {
                    auto equipment = std::static_pointer_cast<fb::game::equipment>(item);
                    equipment->durability(*params.durability);
                }

                if (model.attr(ITEM_ATTRIBUTE::CONSUME))
                {
                    auto consume = std::static_pointer_cast<fb::game::consume>(item);
                    consume->durability(*params.durability);
                }
            }

            if (params.custom_name.has_value() && params.custom_name->empty() == false &&
                *params.custom_name != "null" && model.attr(ITEM_ATTRIBUTE::WEAPON))
            {
                auto weapon = std::static_pointer_cast<fb::game::weapon>(item);
                weapon->custom_name(*params.custom_name);
            }
            this->items.add(item);
            break;
        }
        case fb::model::enum_value::DSL::money:
        {
            auto params  = fb::model::dsl::money(item.params);
            money       += params.value;
            break;
        }
        case fb::model::enum_value::DSL::exp:
        {
            auto params  = fb::model::dsl::exp(item.params);
            exp         += params.value;
            break;
        }
        default:
            break;
        }
    }

    if (exp > 0)
        this->add_exp(exp, false, true);

    if (money > 0)
        this->money_add(money);

    return true;
}

std::shared_ptr<fb::socket<character>> character::socket_ptr() const
{
    return this->_socket.lock();
}

fb::game::character::ping_state_t& character::ping_state()
{
    this->assert_thread();
    return this->_ping_state;
}

std::shared_ptr<fb::game::appearance> character::appearance() const
{
    if (this->_mimicry.has_value())
        return std::make_shared<character_appearance>(this->_mimicry.value());

    auto ptr        = std::make_shared<character_appearance>();
    ptr->gender     = this->_gender;
    ptr->state      = this->_state;
    ptr->hair       = this->_look;
    ptr->hair_color = this->_color;

    if (this->items.weapon() != nullptr)
    {
        ptr->weapon       = this->items.weapon()->based<fb::model::weapon>().dress;
        ptr->weapon_color = this->_weapon_color;
    }

    if (this->items.armor() != nullptr)
    {
        ptr->armor       = this->items.armor()->based<fb::model::armor>().dress;
        ptr->armor_color = this->_armor_color;
    }

    if (this->items.shield() != nullptr)
    {
        ptr->shield       = this->items.shield()->based<fb::model::shield>().dress;
        ptr->shield_color = this->_shield_color;
    }

    return ptr;
}