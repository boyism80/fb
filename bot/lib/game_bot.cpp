#include <fb/bot/game_bot.h>
#include <fb/bot/load/game_controller.h>

using namespace fb::bot;

game_bot::game_bot(bot_controller<game_bot>& bot_controller, uint32_t id) :
    bot<game_bot>(bot_controller, id)
{
    this->_next_action_time = fb::model::datetime();

    this->pattern(&game_bot::pattern_chat, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_attack, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_direction, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_move, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_loot, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_emotion, 250ms, 1000ms);
    // this->pattern(&game_bot::pattern_bulletin_sections, 250ms, 1000ms);
}

game_bot::game_bot(bot_controller<game_bot>& bot_controller, uint32_t id, const fb::stream& params) :
    game_bot(bot_controller, id)
{
    auto clone    = fb::stream{params};
    auto reader   = fb::stream_reader<>(clone);
    auto enc_type = reader.read<uint8_t>();
    auto key_size = reader.read<uint8_t>();
    auto enc_key  = new uint8_t[key_size];
    reader.read(enc_key, key_size);
    this->crt() = fb::crypto(enc_type, enc_key);
    delete[] enc_key;

    this->_transfer_buffer = params;
}

game_bot::~game_bot()
{ }

// Game state accessor implementations
uint32_t game_bot::oid() const
{
    return this->_oid;
}

void game_bot::set_oid(uint32_t value)
{
    this->_oid = value;
}

point<uint16_t> game_bot::position() const
{
    return this->_position;
}

void game_bot::set_position(const point<uint16_t>& value)
{
    this->_position = value;
}

bool game_bot::is_initialized() const
{
    return this->_inited;
}

void game_bot::set_initialized(bool value)
{
    this->_inited = value;
}

const fb::stream& game_bot::transfer_buffer() const
{
    return this->_transfer_buffer;
}

DIRECTION game_bot::direction() const
{
    return this->_direction;
}

void game_bot::set_direction(DIRECTION value)
{
    this->_direction = value;
}

// Additional state accessor implementations
uint16_t game_bot::look() const
{
    return this->_look;
}

void game_bot::set_look(uint16_t value)
{
    this->_look = value;
}

uint8_t game_bot::color() const
{
    return this->_color;
}

void game_bot::set_color(uint8_t value)
{
    this->_color = value;
}

bool game_bot::is_dead() const
{
    return this->_dead;
}

void game_bot::set_dead(bool value)
{
    this->_dead = value;
}

// Buff management method implementations
const std::set<std::string>& game_bot::active_buffs() const
{
    return this->_active_buffs;
}

void game_bot::add_buff(const std::string& name)
{
    this->_active_buffs.insert(name);
}

void game_bot::remove_buff(const std::string& name)
{
    this->_active_buffs.erase(name);
}

bool game_bot::has_buff(const std::string& name) const
{
    return this->_active_buffs.count(name) > 0;
}

// Spell management method implementations
void game_bot::update_spell(uint8_t slot, const std::string& name, uint8_t type)
{
    this->_spells[slot] = simple_spell(name, type);
}

void game_bot::remove_spell(uint8_t slot)
{
    this->_spells.erase(slot);
}

std::optional<game_bot::simple_spell> game_bot::get_spell(uint8_t slot) const
{
    auto it = this->_spells.find(slot);
    if (it != this->_spells.end())
    {
        return it->second;
    }
    return std::nullopt;
}

bool game_bot::has_spell(uint8_t slot) const
{
    return this->_spells.find(slot) != this->_spells.end();
}

const std::map<uint8_t, game_bot::simple_spell>& game_bot::spells() const
{
    return this->_spells;
}

// Character state accessor implementations
uint8_t game_bot::nation() const
{
    return this->_nation;
}

void game_bot::set_nation(uint8_t value)
{
    this->_nation = value;
}

uint8_t game_bot::creature() const
{
    return this->_creature;
}

void game_bot::set_creature(uint8_t value)
{
    this->_creature = value;
}

uint8_t game_bot::level() const
{
    return this->_level;
}

void game_bot::set_level(uint8_t value)
{
    this->_level = value;
}

uint32_t game_bot::base_hp() const
{
    return this->_base_hp;
}

void game_bot::set_base_hp(uint32_t value)
{
    this->_base_hp = value;
}

uint32_t game_bot::base_mp() const
{
    return this->_base_mp;
}

void game_bot::set_base_mp(uint32_t value)
{
    this->_base_mp = value;
}

uint8_t game_bot::strength() const
{
    return this->_strength;
}

void game_bot::set_strength(uint8_t value)
{
    this->_strength = value;
}

uint8_t game_bot::intelligence() const
{
    return this->_intelligence;
}

void game_bot::set_intelligence(uint8_t value)
{
    this->_intelligence = value;
}

uint8_t game_bot::dexterity() const
{
    return this->_dexterity;
}

void game_bot::set_dexterity(uint8_t value)
{
    this->_dexterity = value;
}

uint32_t game_bot::hp() const
{
    return this->_hp;
}

void game_bot::set_hp(uint32_t value)
{
    this->_hp = value;
}

uint32_t game_bot::mp() const
{
    return this->_mp;
}

void game_bot::set_mp(uint32_t value)
{
    this->_mp = value;
}

uint32_t game_bot::exp() const
{
    return this->_exp;
}

void game_bot::set_exp(uint32_t value)
{
    this->_exp = value;
}

uint32_t game_bot::money() const
{
    return this->_money;
}

void game_bot::set_money(uint32_t value)
{
    this->_money = value;
}

uint32_t game_bot::crowd_control() const
{
    return this->_crowd_control;
}

void game_bot::set_crowd_control(uint32_t value)
{
    this->_crowd_control = value;
}

uint8_t game_bot::mail_count() const
{
    return this->_mail_count;
}

void game_bot::set_mail_count(uint8_t value)
{
    this->_mail_count = value;
}

uint8_t game_bot::fast_move() const
{
    return this->_fast_move;
}

void game_bot::set_fast_move(uint8_t value)
{
    this->_fast_move = value;
}

uint8_t game_bot::disguised() const
{
    return this->_disguised;
}

void game_bot::set_disguised(uint8_t value)
{
    this->_disguised = value;
}

SEX game_bot::sex() const
{
    return this->_sex;
}

void game_bot::set_sex(SEX value)
{
    this->_sex = value;
}

STATE game_bot::state() const
{
    return this->_state;
}

void game_bot::set_state(STATE value)
{
    this->_state = value;
}

uint8_t game_bot::armor_dress() const
{
    return this->_armor_dress;
}

void game_bot::set_armor_dress(uint8_t value)
{
    this->_armor_dress = value;
}

uint8_t game_bot::armor_color() const
{
    return this->_armor_color;
}

void game_bot::set_armor_color(uint8_t value)
{
    this->_armor_color = value;
}

uint16_t game_bot::weapon_dress() const
{
    return this->_weapon_dress;
}

void game_bot::set_weapon_dress(uint16_t value)
{
    this->_weapon_dress = value;
}

uint8_t game_bot::weapon_color() const
{
    return this->_weapon_color;
}

void game_bot::set_weapon_color(uint8_t value)
{
    this->_weapon_color = value;
}

uint8_t game_bot::shield_dress() const
{
    return this->_shield_dress;
}

void game_bot::set_shield_dress(uint8_t value)
{
    this->_shield_dress = value;
}

uint8_t game_bot::shield_color() const
{
    return this->_shield_color;
}

void game_bot::set_shield_color(uint8_t value)
{
    this->_shield_color = value;
}

uint8_t game_bot::head_marker() const
{
    return this->_head_marker;
}

void game_bot::set_head_marker(uint8_t value)
{
    this->_head_marker = value;
}

const std::string& game_bot::name() const
{
    return this->_name;
}

void game_bot::set_name(const std::string& value)
{
    this->_name = value;
}

async::task<void> game_bot::move(DIRECTION direction, int step, const fb::model::timespan& delay)
{
    auto thread = this->thread();
    auto before = this->_position;
    auto after  = before;
    for (int i = 0; i < step; i++)
    {
        this->send(fb::protocol::game::request::move{direction, this->_oid, after});
        switch (direction)
        {
        case DIRECTION::LEFT:
            after.x--;
            break;

        case DIRECTION::TOP:
            after.y--;
            break;

        case DIRECTION::RIGHT:
            after.x++;
            break;

        case DIRECTION::BOTTOM:
            after.y++;
            break;
        }

        this->_position = after;
        co_await this->thread()->sleep(delay);
    }
}

async::task<void>
game_bot::map_move(const std::string& map_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout)
{
    auto command = std::format("/맵이동 {} {} {}", map_name, x, y);
    std::ignore  = co_await this->request<fb::protocol::game::response::position>(
        fb::protocol::game::request::chat{false, command},
        timeout);
}

async::task<void> game_bot::change_level(uint8_t level, std::chrono::milliseconds timeout)
{
    auto command = std::format("/레벨바꾸기 {}", level);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [level](auto& resp) -> bool {
            return ENUM_IN(resp.level, STATE_LEVEL::BASED) && resp.ch_level == level;
        },
        timeout);
}

async::task<void>
game_bot::change_stats(uint8_t str, uint8_t dex, uint8_t intelligence, std::chrono::milliseconds timeout)
{
    auto command = std::format("/스탯바꾸기 {} {} {}", str, dex, intelligence);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [str, dex, intelligence](auto& resp) -> bool {
            return resp.ch_strength == str && resp.ch_dexterity == dex && resp.ch_intelligence == intelligence;
        },
        timeout);
}

async::task<void> game_bot::change_sex(SEX sex, std::chrono::milliseconds timeout)
{
    auto command = std::format("/성별바꾸기 {}", (uint8_t)sex);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_external<true>>(
        fb::protocol::game::request::chat{false, command},
        [sex](auto& resp) -> bool {
            return resp.sex == sex;
        },
        timeout);
}

async::task<void> game_bot::change_base_hp(uint32_t hp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/체력바꾸기 {}", hp);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [hp](auto& resp) -> bool {
            return resp.ch_base_hp == hp;
        },
        timeout);
}

async::task<void> game_bot::change_base_mp(uint32_t mp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/마력바꾸기 {}", mp);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [mp](auto& resp) -> bool {
            return resp.ch_base_mp == mp;
        },
        timeout);
}

async::task<void> game_bot::change_hp(uint32_t hp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/현재체력 {}", hp);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [hp](auto& resp) -> bool {
            return resp.ch_hp == hp;
        },
        timeout);
}

async::task<void> game_bot::change_mp(uint32_t mp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/현재마력 {}", mp);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [mp](auto& resp) -> bool {
            return resp.ch_mp == mp;
        },
        timeout);
}

void game_bot::direction(DIRECTION direction)
{
    this->send(fb::protocol::game::request::direction{direction});
}

async::task<void> game_bot::process_random_pattern(const fb::model::datetime& now)
{
    if (!this->_inited)
        co_return;

    if (now < this->_next_action_time)
        co_return;

    static std::random_device       device;
    static std::mt19937             gen(device());
    std::uniform_int_distribution<> dist(0, this->_pattern_params.size() - 1);

    auto& pattern  = this->_pattern_params.at(dist(gen));
    auto  datetime = fb::model::datetime{now};
    co_await pattern.fn();

    auto rand_term          = std::uniform_int_distribution<long long>(pattern.min.count(), pattern.max.count())(gen);
    this->_next_action_time = datetime + std::chrono::steady_clock::duration(rand_term);
}

async::task<void> game_bot::pattern_chat()
{
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::vector<std::string>        messages{"안녕하세요", "반갑습니다."};
    static std::uniform_int_distribution<> dist(0, messages.size() - 1);

    auto& message = messages.at(dist(gen));
    this->chat(message);
    co_return;
}

async::task<void> game_bot::pattern_attack()
{
    this->send(fb::protocol::game::request::attack());
    co_return;
}

async::task<void> game_bot::pattern_direction()
{
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device     device;
    static std::mt19937           gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::direction{direction});

    // Update internal direction state
    this->_direction = direction;

    co_return;
}

async::task<void> game_bot::pattern_move()
{
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device     device;
    static std::mt19937           gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::move{direction, this->_oid, this->_position});

    // Update internal direction state
    this->_direction = direction;

    switch (direction)
    {
    case DIRECTION::LEFT:
        this->_position.x--;
        break;

    case DIRECTION::TOP:
        this->_position.y--;
        break;

    case DIRECTION::RIGHT:
        this->_position.x++;
        break;

    case DIRECTION::BOTTOM:
        this->_position.y++;
        break;
    }
    co_return;
}

async::task<void> game_bot::pattern_loot()
{
    this->send(fb::protocol::game::request::loot{false});
    co_return;
}

async::task<void> game_bot::pattern_emotion()
{
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::uniform_int_distribution<> dist(0, 0xFF - 0x0B);

    this->send(fb::protocol::game::request::emotion{(uint8_t)dist(gen)});
    co_return;
}

async::task<void> game_bot::pattern_bulletin_sections()
{
    std::random_device              device;
    std::mt19937                    gen(device());
    std::uniform_int_distribution<> dist(0, 1);

    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::SECTIONS));

    auto section = (uint32_t)dist(gen);
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLES, section));
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLE, section, 0));
    this->send(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::WRITE, section, 0, 0, "게시글 타이틀", "게시글 내용"));
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::DELETE, section, 0));
    co_return;
}

// Simple item management method implementations
void game_bot::update_item(uint8_t slot, const std::string& name, uint32_t count)
{
    this->_items[slot] = simple_item(name, count);
}

void game_bot::remove_item(uint8_t slot)
{
    this->_items.erase(slot);
}

std::optional<game_bot::simple_item> game_bot::get_item(uint8_t slot) const
{
    auto it = this->_items.find(slot);
    if (it != this->_items.end())
    {
        return it->second;
    }
    return std::nullopt;
}

bool game_bot::has_item(uint8_t slot) const
{
    return this->_items.find(slot) != this->_items.end();
}

const std::map<uint8_t, game_bot::simple_item>& game_bot::items() const
{
    return this->_items;
}

bool game_bot::has_item_by_name(const std::string& name) const
{
    for (const auto& [slot, item] : this->_items)
    {
        if (item.name.find(name) != std::string::npos)
            return true;
    }
    return false;
}

uint16_t game_bot::get_item_count_by_name(const std::string& name) const
{
    for (const auto& [slot, item] : this->_items)
    {
        if (item.name.find(name) != std::string::npos)
            return item.count;
    }
    return 0;
}

uint8_t game_bot::get_item_slot_by_name(const std::string& name) const
{
    for (const auto& [slot, item] : this->_items)
    {
        if (item.name.find(name) != std::string::npos)
            return slot;
    }
    return 0xFF;
}

void game_bot::remove_buffs()
{
    this->send(fb::protocol::game::request::chat{false, "/버프해제"});
}

void game_bot::chat(const std::string& message)
{
    this->send(fb::protocol::game::request::chat{false, message});
}

async::task<void> game_bot::create_item(const std::string& item_name, uint32_t count, std::chrono::milliseconds timeout)
{
    auto command = std::format("/아이템생성 {} {}", item_name, count);
    std::ignore  = co_await this->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::chat{false, command},
        [item_name, count](auto& resp) -> bool {
            return resp.name.starts_with(item_name) && resp.count == count;
        },
        timeout);
}

async::task<void> game_bot::change_money(uint32_t amount, std::chrono::milliseconds timeout)
{
    auto command = std::format("/금전 {}", amount);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        [amount](auto& resp) -> bool {
            return resp.ch_money == amount;
        },
        timeout);
}

async::task<void> game_bot::drop_item(uint8_t index, bool all, std::chrono::milliseconds timeout)
{
    std::ignore = co_await this->request<fb::protocol::game::response::item_remove>(
        fb::protocol::game::request::item_drop(index + 1, all),
        [index, all](auto& resp) -> bool {
            return resp.type == ITEM_DELETE_TYPE::DROP && resp.index == index;
        },
        timeout);
}

async::task<void> game_bot::drop_money(uint32_t amount, std::chrono::milliseconds timeout)
{
    auto before = this->money();
    std::ignore = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::item_drop_money(amount),
        [before, amount](auto& resp) -> bool {
            return resp.ch_money == before - amount;
        },
        timeout);
}

async::task<bool> game_bot::equip(uint8_t slot, const std::string& expected_msg, std::chrono::milliseconds timeout)
{
    try
    {
        auto&& resp = co_await this->request<fb::protocol::game::response::message>(
            fb::protocol::game::request::item_active(slot + 1),
            [](auto& resp) -> bool {
                return resp.type == MESSAGE_TYPE::STATE;
            },
            timeout);

        co_return resp.text.find(expected_msg) != std::string::npos;
    }
    catch (const std::exception& e)
    {
        co_return false;
    }
}

async::task<bool> game_bot::unequip(EQUIPMENT_PARTS parts, std::chrono::milliseconds timeout)
{
    try
    {
        std::ignore = co_await this->request<fb::protocol::game::response::item_update>(
            fb::protocol::game::request::item_inactive(parts),
            timeout);

        co_return true;
    }
    catch (const std::exception& e)
    {
        co_return false;
    }
}

async::task<void> game_bot::sleep(std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    if (thread == nullptr)
        co_return;

    co_await thread->switching();
    co_await thread->sleep(timeout);
}

async::task<void> game_bot::change_class(const std::string& class_name, std::chrono::milliseconds timeout)
{
    auto command = std::format("/직업바꾸기 {}", class_name);
    std::ignore  = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, command},
        timeout);
}

async::task<spawned_monster_info>
game_bot::spawn_monster_with_validator(std::shared_ptr<fb::bot::game_bot>                               bot,
                                       const std::string&                                               monster_name,
                                       uint16_t                                                         x,
                                       uint16_t                                                         y,
                                       std::function<bool(const fb::protocol::game::response::update&)> validator,
                                       std::chrono::milliseconds                                        timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto&& spawn_response = co_await this->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
        validator,
        timeout);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(std::format("Failed to spawn monster {} at ({}, {})", monster_name, x, y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<spawned_monster_info> game_bot::spawn_monster_by_look(const std::string&        monster_name,
                                                                  uint16_t                  x,
                                                                  uint16_t                  y,
                                                                  uint32_t                  expected_look,
                                                                  std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto&& spawn_response = co_await this->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
        [expected_look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;

            auto& mob = resp.objects_data.front();
            return mob.look == expected_look;
        },
        timeout);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(
            std::format("Failed to spawn monster {} with look {} at ({}, {})", monster_name, expected_look, x, y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<void> game_bot::spawn_monsters_by_look_bulk(const std::string&        monster_name,
                                                        uint8_t                   range,
                                                        uint32_t                  expected_look,
                                                        std::chrono::milliseconds timeout)
{
    co_await this->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false, std::format("/몬스터범위생성 {} {}", monster_name, range)},
        [expected_look](auto& resp) -> bool {
            for (const auto& mob : resp.objects_data)
            {
                if (mob.look != expected_look)
                    return false;
            }
            return true;
        },
        timeout);
}

async::task<std::vector<spawned_monster_info>> game_bot::spawn_monsters_relative_with_validator(
    std::shared_ptr<fb::bot::game_bot>                               bot,
    const std::string&                                               monster_name,
    const std::vector<std::pair<int, int>>&                          relative_positions,
    std::function<bool(const fb::protocol::game::response::update&)> validator,
    std::chrono::milliseconds                                        timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto                              caster_pos = this->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await this->request<fb::protocol::game::response::update>(
            fb::protocol::game::request::chat{false,
                                              std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
            validator,
            timeout);

        if (spawn_response.objects_data.empty())
        {
            throw std::runtime_error(
                std::format("Failed to spawn monster {} at ({}, {})", monster_name, monster_x, monster_y));
        }

        auto&                mob = spawn_response.objects_data.front();
        spawned_monster_info monster_info;
        monster_info.oid      = mob.oid;
        monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
        monster_info.look     = mob.look;
        spawned_monsters.push_back(monster_info);
    }

    co_return spawned_monsters;
}

async::task<std::vector<spawned_monster_info>>
game_bot::spawn_monsters_relative_by_look(const std::string&                      monster_name,
                                          const std::vector<std::pair<int, int>>& relative_positions,
                                          uint32_t                                expected_look,
                                          std::chrono::milliseconds               timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto                              caster_pos = this->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await this->request<fb::protocol::game::response::update>(
            fb::protocol::game::request::chat{false,
                                              std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
            [expected_look](auto& resp) -> bool {
                if (resp.objects_data.empty())
                    return false;
                auto& mob = resp.objects_data.front();
                return mob.look == expected_look;
            },
            timeout);

        if (spawn_response.objects_data.empty())
        {
            throw std::runtime_error(std::format("Failed to spawn monster {} with look {} at ({}, {})",
                                                 monster_name,
                                                 expected_look,
                                                 monster_x,
                                                 monster_y));
        }

        auto&                mob = spawn_response.objects_data.front();
        spawned_monster_info monster_info;
        monster_info.oid      = mob.oid;
        monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
        monster_info.look     = mob.look;
        spawned_monsters.push_back(monster_info);
    }

    co_return spawned_monsters;
}

async::task<spawned_monster_info> game_bot::spawn_monster_relative_by_look(const std::string&        monster_name,
                                                                           int                       relative_x,
                                                                           int                       relative_y,
                                                                           uint32_t                  expected_look,
                                                                           std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto caster_pos = this->position();
    auto monster_x  = caster_pos.x + relative_x;
    auto monster_y  = caster_pos.y + relative_y;

    auto&& spawn_response = co_await this->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false,
                                          std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
        [expected_look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;
            auto& mob = resp.objects_data.front();
            return mob.look == expected_look;
        },
        timeout);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(std::format("Failed to spawn monster {} with look {} at relative position ({}, {})",
                                             monster_name,
                                             expected_look,
                                             relative_x,
                                             relative_y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<bool> game_bot::set_max_hp_mp(int max_hp, int max_mp, std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto hp_result = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        timeout);

    auto mp_result = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        timeout);

    co_return true; // Both commands should succeed if bot is valid
}

async::task<bool> game_bot::set_current_hp_mp(int current_hp, int current_mp, std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto hp_result = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/현재체력 {}", current_hp)},
        [current_hp](auto& resp) -> bool {
            return resp.ch_hp == current_hp;
        },
        timeout);

    auto mp_result = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/현재마력 {}", current_mp)},
        [current_mp](auto& resp) -> bool {
            return resp.ch_mp == current_mp;
        },
        timeout);

    co_return true; // Both commands should succeed if bot is valid
}

async::task<bool> game_bot::setup_bot_stats(int                       max_hp,
                                            int                       max_mp,
                                            std::optional<int>        current_hp,
                                            std::optional<int>        current_mp,
                                            std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    // Set max HP/MP
    std::ignore = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        timeout);

    std::ignore = co_await this->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        timeout);

    // Set current HP/MP if specified
    if (current_hp.has_value())
    {
        std::ignore = co_await this->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재체력 {}", current_hp.value())},
            [current_hp](auto& resp) -> bool {
                return resp.ch_hp == current_hp.value();
            },
            timeout);
    }

    if (current_mp.has_value())
    {
        std::ignore = co_await this->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재마력 {}", current_mp.value())},
            [current_mp](auto& resp) -> bool {
                return resp.ch_mp == current_mp.value();
            },
            timeout);
    }

    co_return true;
}

async::task<uint8_t> game_bot::lean_spell(const std::string& spell_name, std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<fb::protocol::game::response::spell_update>(
        fb::protocol::game::request::chat{false, std::format("/마법배우기 {}", spell_name)},
        [](auto& resp) -> bool {
            return resp.index != 0xFF;
        },
        timeout);

    co_return resp.index;
}

async::task<size_t> game_bot::learn_spells(const std::vector<std::string>& spell_names,
                                           std::chrono::milliseconds       timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    size_t learned_count = 0;
    for (const auto& spell_name : spell_names)
    {
        auto index = co_await this->lean_spell(spell_name, timeout);
        if (index != 0xFF)
        {
            learned_count++;
        }
        else
        {
            fb::logger::warn("Failed to learn spell: {}", spell_name);
        }
    }

    co_return learned_count;
}

async::task<void> game_bot::clear_all_spells(std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto count = this->spells().size();
    if (count == 0)
        co_return;

    auto last_slot = uint8_t{0};
    for (auto& [slot, spell] : this->spells())
    {
        last_slot = std::max<uint8_t>(last_slot, slot);
    }

    this->send(fb::protocol::game::request::chat{false, "/마법지우기"});
    co_return; // Command should succeed if bot is valid
}

async::task<void> game_bot::clear_all_items(std::chrono::milliseconds timeout)
{
    auto thread = this->thread();
    co_await thread->switching();

    this->send(fb::protocol::game::request::chat{false, "/아이템삭제"});
    co_return;
}

async::task<void> game_bot::move_bot_back_to_position(const fb::model::point<uint16_t>& original_position,
                                                      std::chrono::milliseconds         interval)
{
    auto thread = this->thread();
    co_await thread->switching();

    auto current_position = this->position();
    auto move_y_axis      = current_position.y - original_position.y;

    if (move_y_axis > 0)
    {
        for (auto i = 0; i < move_y_axis; i++)
        {
            this->send(fb::protocol::game::request::move{DIRECTION::TOP, this->oid(), current_position});
            co_await thread->sleep(interval);
            current_position.y--;
            this->set_position(current_position);
        }
        this->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});
    }
}