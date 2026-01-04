#include <fb/bot/game_bot.h>
#include <fb/bot/load/game_controller.h>

using namespace fb::bot;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

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
    auto enc_key  = std::make_unique<uint8_t[]>(key_size);
    reader.read(enc_key.get(), key_size);
    this->encryption(enc_type, enc_key.get());
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

uint16_t game_bot::map() const
{
    return this->_map;
}

void game_bot::set_map(uint16_t value)
{
    this->_map = value;
}

point<uint16_t> game_bot::position() const
{
    return this->_position;
}

void game_bot::set_position(const point<uint16_t>& value)
{
    this->_position = value;
}

bool game_bot::inited() const
{
    return this->_inited;
}

void game_bot::inited(bool value)
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

GENDER game_bot::gender() const
{
    return this->_sex;
}

void game_bot::set_sex(GENDER value)
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

const std::string& game_bot::clan_name() const
{
    return this->_clan_name;
}

void game_bot::set_clan_name(const std::string& value)
{
    this->_clan_name = value;
}

const std::string& game_bot::clan_title() const
{
    return this->_clan_title;
}

void game_bot::set_clan_title(const std::string& value)
{
    this->_clan_title = value;
}

const std::string& game_bot::title() const
{
    return this->_title;
}

void game_bot::set_title(const std::string& value)
{
    this->_title = value;
}

const std::string& game_bot::group_info() const
{
    return this->_group_info;
}

void game_bot::set_group_info(const std::string& value)
{
    this->_group_info = value;
}

uint8_t game_bot::group_option() const
{
    return this->_group_option;
}

void game_bot::set_group_option(uint8_t value)
{
    this->_group_option = value;
}

uint32_t game_bot::remained_exp() const
{
    return this->_remained_exp;
}

void game_bot::set_remained_exp(uint32_t value)
{
    this->_remained_exp = value;
}

async::task<void> game_bot::move(DIRECTION direction, int step, const fb::model::timespan& delay)
{
    auto thread = this->thread();
    auto before = this->_position;
    auto after  = before;
    for (int i = 0; i < step; i++)
    {
        this->_direction = direction;
        this->send(game_reqs::move{direction, this->_oid, after});
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

        this->set_position(after);
    }
    co_await this->thread()->sleep(delay);
}

async::task<void>
game_bot::map_move(const std::string& map_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout)
{
    auto command = std::format("/맵이동 {} {} {}", map_name, x, y);
    auto map     = table::map.name2map(map_name);
    if (map == nullptr)
    {
        co_return;
    }
    else if (this->_map == map->id && this->_position == fb::model::point<uint16_t>{x, y})
    {
        co_return;
    }
    else if (this->_map != map->id)
    {
        std::ignore = co_await this->request<game_resp::map_config>(
            game_reqs::chat{false, command},
            [map](auto& resp) -> bool {
                return resp.id == map->id;
            },
            timeout);
    }
    else
    {
        std::ignore = co_await this->request<game_resp::position>(
            game_reqs::chat{false, command},
            [x, y](auto& resp) -> bool {
                return resp.abs.x == x && resp.abs.y == y;
            },
            timeout);
    }
}

async::task<void> game_bot::change_level(uint8_t level, std::chrono::milliseconds timeout)
{
    auto command = std::format("/레벨바꾸기 {}", level);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [level](auto& resp) -> bool {
            return ENUM_IN(resp.level, UPDATE_STATE_LEVEL::BASED) && resp.ch_level == level;
        },
        timeout);
}

async::task<void>
game_bot::change_stats(uint8_t str, uint8_t dex, uint8_t intelligence, std::chrono::milliseconds timeout)
{
    auto command = std::format("/스탯바꾸기 {} {} {}", str, dex, intelligence);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [str, dex, intelligence](auto& resp) -> bool {
            return resp.ch_strength == str && resp.ch_dexterity == dex && resp.ch_intelligence == intelligence;
        },
        timeout);
}

async::task<void> game_bot::change_str(uint8_t str, std::chrono::milliseconds timeout)
{
    auto command = std::format("/힘바꾸기 {}", str);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [str](auto& resp) -> bool {
            return resp.ch_strength == str;
        },
        timeout);
}

async::task<void> game_bot::change_dex(uint8_t dex, std::chrono::milliseconds timeout)
{
    auto command = std::format("/민첩바꾸기 {}", dex);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [dex](auto& resp) -> bool {
            return resp.ch_dexterity == dex;
        },
        timeout);
}

async::task<void> game_bot::change_int(uint8_t intelligence, std::chrono::milliseconds timeout)
{
    auto command = std::format("/지력바꾸기 {}", intelligence);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [intelligence](auto& resp) -> bool {
            return resp.ch_intelligence == intelligence;
        },
        timeout);
}

async::task<void> game_bot::change_sex(GENDER gender, std::chrono::milliseconds timeout)
{
    auto command = std::format("/성별바꾸기 {}", (uint8_t)gender);
    std::ignore  = co_await this->request<game_resp::update_external<true>>(
        game_reqs::chat{false, command},
        [gender](auto& resp) -> bool {
            return resp.gender == gender;
        },
        timeout);
}

async::task<void> game_bot::change_base_hp(uint32_t hp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/체력바꾸기 {}", hp);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [hp](auto& resp) -> bool {
            return resp.ch_base_hp == hp;
        },
        timeout);
}

async::task<void> game_bot::change_base_mp(uint32_t mp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/마력바꾸기 {}", mp);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [mp](auto& resp) -> bool {
            return resp.ch_base_mp == mp;
        },
        timeout);
}

async::task<void> game_bot::change_hp(uint32_t hp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/현재체력 {}", hp);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [hp](auto& resp) -> bool {
            return resp.ch_hp == hp;
        },
        timeout);
}

async::task<void> game_bot::change_mp(uint32_t mp, std::chrono::milliseconds timeout)
{
    auto command = std::format("/현재마력 {}", mp);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [mp](auto& resp) -> bool {
            return resp.ch_mp == mp;
        },
        timeout);
}

async::task<void> game_bot::direction(DIRECTION direction, std::chrono::milliseconds timeout)
{
    if (this->_direction == direction)
        co_return;

    try
    {
        std::ignore = co_await this->request<game_resp::direction>(
            game_reqs::direction{direction},
            [direction](auto& resp) -> bool {
                return resp.value == direction;
            },
            timeout);
    }
    catch (std::exception&)
    { }
}

async::task<void> game_bot::process_random_pattern(const fb::model::datetime& now)
{
    if (!this->_inited)
        co_return;

    if (now < this->_next_action_time)
        co_return;

    static auto device = std::random_device();
    static auto gen    = std::mt19937(device());
    static auto dist   = std::uniform_int_distribution<>(0, this->_pattern_params.size() - 1);

    auto& pattern  = this->_pattern_params.at(dist(gen));
    auto  datetime = fb::model::datetime{now};
    co_await pattern.fn();

    auto rand_term          = std::uniform_int_distribution<long long>(pattern.min.count(), pattern.max.count())(gen);
    this->_next_action_time = datetime + std::chrono::steady_clock::duration(rand_term);
}

async::task<void> game_bot::pattern_chat()
{
    static auto device   = std::random_device();
    static auto gen      = std::mt19937(device());
    static auto messages = std::vector<std::string>{"안녕하세요", "반갑습니다."};
    static auto dist     = std::uniform_int_distribution<>(0, messages.size() - 1);

    auto& message = messages.at(dist(gen));
    this->chat(message);
    co_return;
}

async::task<void> game_bot::pattern_attack()
{
    this->send(game_reqs::attack());
    co_return;
}

async::task<void> game_bot::pattern_direction()
{
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static auto                   device = std::random_device();
    static auto                   gen    = std::mt19937(device());
    static auto                   dist   = std::uniform_int_distribution<>(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(game_reqs::direction{direction});

    // Update internal direction state
    this->_direction = direction;

    co_return;
}

async::task<void> game_bot::pattern_move()
{
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static auto                   device = std::random_device();
    static auto                   gen    = std::mt19937(device());
    static auto                   dist   = std::uniform_int_distribution<>(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(game_reqs::move{direction, this->_oid, this->_position});

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
    this->send(game_reqs::loot{false});
    co_return;
}

async::task<void> game_bot::pattern_emotion()
{
    static auto device = std::random_device();
    static auto gen    = std::mt19937(device());
    static auto dist   = std::uniform_int_distribution<>(0, 0xFF - 0x0B);

    this->send(game_reqs::emotion{(uint8_t)dist(gen)});
    co_return;
}

async::task<void> game_bot::pattern_bulletin_sections()
{
    static auto device = std::random_device();
    static auto gen    = std::mt19937(device());
    static auto dist   = std::uniform_int_distribution<>(0, 1);

    this->send(game_reqs::bulletin(BULLETIN_ACTION::SECTIONS));

    auto section = (uint32_t)dist(gen);
    this->send(game_reqs::bulletin(BULLETIN_ACTION::ARTICLES, section));
    this->send(game_reqs::bulletin(BULLETIN_ACTION::ARTICLE, section, 0));
    this->send(game_reqs::bulletin(BULLETIN_ACTION::WRITE, section, 0, 0, "게시글 타이틀", "게시글 내용"));
    this->send(game_reqs::bulletin(BULLETIN_ACTION::DELETE, section, 0));
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

std::optional<game_bot::simple_spell> game_bot::get_spell(uint8_t slot) const
{
    auto it = this->_spells.find(slot);
    if (it != this->_spells.end())
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
    auto it = std::find_if(this->_items.begin(), this->_items.end(), [&name](const auto& item) {
        return item.second.name.find(name) != std::string::npos;
    });
    return it != this->_items.end() ? it->first : 0xFF;
}

uint8_t game_bot::get_spell_slot_by_name(const std::string& name) const
{
    auto it = std::find_if(this->_spells.begin(), this->_spells.end(), [&name](const auto& spell) {
        return spell.second.name.find(name) != std::string::npos;
    });
    return it != this->_spells.end() ? it->first : 0xFF;
}

void game_bot::remove_buffs()
{
    this->send(game_reqs::chat{false, "/버프해제"});
}

void game_bot::chat(const std::string& message)
{
    this->send(game_reqs::chat{false, message});
}

async::task<void> game_bot::create_item(const std::string& item_name, uint32_t count, std::chrono::milliseconds timeout)
{
    auto command = std::format("/아이템생성 {} {}", item_name, count);
    std::ignore  = co_await this->request<game_resp::item_update>(
        game_reqs::chat{false, command},
        [item_name, count](auto& resp) -> bool {
            return resp.name.starts_with(item_name) && resp.count == count;
        },
        timeout);
}

async::task<game_bot::simple_npc> game_bot::create_npc(const std::string& npc_name, std::chrono::milliseconds timeout)
{
    auto command = std::format("/엔피씨생성 {}", npc_name);
    auto model   = table::npc.name2npc(npc_name);
    if (!model)
        throw std::runtime_error(std::format("Failed to find NPC model for {}", npc_name));

    auto   look = model->look;
    auto&& resp = co_await this->request<game_resp::update>(
        game_reqs::chat{false, command},
        [bot_pos = this->position(), look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;

            auto& npc = resp.objects_data.front();
            if (npc.look != look)
                return false;

            if (npc.x != bot_pos.x)
                return false;

            if (npc.y != bot_pos.y)
                return false;

            return true;
        },
        timeout);

    auto& npc = resp.objects_data.front();
    co_return simple_npc(npc.oid, npc_name);
}

async::task<void> game_bot::change_money(uint32_t amount, std::chrono::milliseconds timeout)
{
    auto command = std::format("/금전 {}", amount);
    std::ignore  = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, command},
        [amount](auto& resp) -> bool {
            return resp.ch_money == amount;
        },
        timeout);
}

async::task<void> game_bot::drop_item(uint8_t index, bool all, std::chrono::milliseconds timeout)
{
    std::ignore = co_await this->request<game_resp::item_remove>(
        game_reqs::item_drop(index + 1, all),
        [index, all](auto& resp) -> bool {
            return resp.type == ITEM_DELETE_TYPE::DROP && resp.index == index;
        },
        timeout);
}

async::task<void> game_bot::drop_money(uint32_t amount, std::chrono::milliseconds timeout)
{
    auto before = this->money();
    std::ignore = co_await this->request<game_resp::update_internal>(
        game_reqs::item_drop_money(amount),
        [before, amount](auto& resp) -> bool {
            return resp.ch_money == before - amount;
        },
        timeout);
}

async::task<bool> game_bot::equip(uint8_t slot, std::chrono::milliseconds timeout)
{
    static const auto prefix_map = std::unordered_map<fb::model::enum_value::ITEM_TYPE, std::string>{
        {ITEM_TYPE::WEAPON,    "w:무기  :" },
        {ITEM_TYPE::ARMOR,     "a:갑옷  :" },
        {ITEM_TYPE::SHIELD,    "s:방패  :" },
        {ITEM_TYPE::HELMET,    "h:머리  :" },
        {ITEM_TYPE::RING,      "l:왼손  :" },
        {ITEM_TYPE::AUXILIARY, "[:보조1  :"}
    };

    try
    {
        if (this->_items.contains(slot) == false)
        {
            fb::logger::fatal("Failed to find item in slot {}", slot);
            co_return false;
        }

        auto& item       = this->_items.at(slot);
        auto  item_model = table::item.name2item(item.name);
        if (!item_model)
        {
            fb::logger::fatal("Failed to find item model for {}", item.name);
            co_return false;
        }

        auto   prefix = prefix_map.at(item_model->type);
        auto&& resp   = co_await this->request<game_resp::message>(
            game_reqs::item_active(slot),
            [prefix](auto& resp) -> bool {
                return resp.type == MESSAGE_TYPE::STATE;
            },
            timeout);

        co_return resp.text.find(prefix) != std::string::npos;
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
        std::ignore = co_await this->request<game_resp::item_update>(game_reqs::item_inactive(parts), timeout);

        co_return true;
    }
    catch (const std::exception& e)
    {
        co_return false;
    }
}

async::task<void> game_bot::sleep(std::chrono::milliseconds timeout)
{
    co_await this->thread()->sleep(timeout);
}

async::task<void> game_bot::change_class(const std::string& class_name, std::chrono::milliseconds timeout)
{
    auto command = std::format("/직업바꾸기 {}", class_name);
    std::ignore  = co_await this->request<game_resp::update_internal>(game_reqs::chat{false, command}, timeout);
}

async::task<spawned_monster_info>
game_bot::spawn_monster_with_validator(std::shared_ptr<fb::bot::game_bot>            bot,
                                       const std::string&                            monster_name,
                                       uint16_t                                      x,
                                       uint16_t                                      y,
                                       std::function<bool(const game_resp::update&)> validator,
                                       std::chrono::milliseconds                     timeout)
{

    auto&& spawn_response = co_await this->request<game_resp::update>(
        game_reqs::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
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

async::task<spawned_monster_info>
game_bot::spawn_monster(const std::string& monster_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout)
{
    auto expected_look = table::mob.name2mob(monster_name)->look;

    auto&& spawn_response = co_await this->request<game_resp::update>(
        game_reqs::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
        [expected_look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;

            auto& mob = resp.objects_data.front();
            return mob.look == expected_look;
        },
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

async::task<void> game_bot::spawn_monsters_bulk(const std::string&        monster_name,
                                                uint8_t                   range,
                                                std::chrono::milliseconds timeout)
{
    auto expected_look = table::mob.name2mob(monster_name)->look;

    std::ignore = co_await this->request<game_resp::update>(
        game_reqs::chat{false, std::format("/몬스터범위생성 {} {}", monster_name, range)},
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

async::task<std::vector<spawned_monster_info>>
game_bot::spawn_monsters_relative_with_validator(std::shared_ptr<fb::bot::game_bot>            bot,
                                                 const std::string&                            monster_name,
                                                 const std::vector<std::pair<int, int>>&       relative_positions,
                                                 std::function<bool(const game_resp::update&)> validator,
                                                 std::chrono::milliseconds                     timeout)
{

    auto                              caster_pos = this->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await this->request<game_resp::update>(
            game_reqs::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
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
game_bot::spawn_monsters_relative(const std::string&                      monster_name,
                                  const std::vector<std::pair<int, int>>& relative_positions,
                                  std::chrono::milliseconds               timeout)
{
    auto expected_look = table::mob.name2mob(monster_name)->look;

    auto                              caster_pos = this->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await this->request<game_resp::update>(
            game_reqs::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
            [expected_look, monster_x, monster_y](auto& resp) -> bool {
                if (resp.objects_data.empty())
                    return false;
                auto& mob = resp.objects_data.front();
                if (mob.look != expected_look)
                    return false;

                if (mob.x != monster_x)
                    return false;

                if (mob.y != monster_y)
                    return false;

                return true;
            },
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

async::task<spawned_monster_info> game_bot::spawn_monster_relative(const std::string&        monster_name,
                                                                   int                       relative_x,
                                                                   int                       relative_y,
                                                                   std::chrono::milliseconds timeout)
{
    auto expected_look = table::mob.name2mob(monster_name)->look;

    auto caster_pos = this->position();
    auto monster_x  = caster_pos.x + relative_x;
    auto monster_y  = caster_pos.y + relative_y;

    auto&& spawn_response = co_await this->request<game_resp::update>(
        game_reqs::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
        [expected_look, monster_x, monster_y](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;
            auto& mob = resp.objects_data.front();
            if (mob.look != expected_look)
                return false;

            if (mob.x != monster_x)
                return false;

            if (mob.y != monster_y)
                return false;

            return true;
        },
        timeout);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(std::format("Failed to spawn monster {} at relative position ({}, {})",
                                             monster_name,
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

    auto hp_result = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        timeout);

    auto mp_result = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        timeout);

    co_return true; // Both commands should succeed if bot is valid
}

async::task<bool> game_bot::set_current_hp_mp(int current_hp, int current_mp, std::chrono::milliseconds timeout)
{
    auto hp_result = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/현재체력 {}", current_hp)},
        [current_hp](auto& resp) -> bool {
            return resp.ch_hp == current_hp;
        },
        timeout);

    auto mp_result = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/현재마력 {}", current_mp)},
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
    // Set max HP/MP
    std::ignore = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        timeout);

    std::ignore = co_await this->request<game_resp::update_internal>(
        game_reqs::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        timeout);

    // Set current HP/MP if specified
    if (current_hp.has_value())
    {
        std::ignore = co_await this->request<game_resp::update_internal>(
            game_reqs::chat{false, std::format("/현재체력 {}", current_hp.value())},
            [current_hp](auto& resp) -> bool {
                return resp.ch_hp == current_hp.value();
            },
            timeout);
    }

    if (current_mp.has_value())
    {
        std::ignore = co_await this->request<game_resp::update_internal>(
            game_reqs::chat{false, std::format("/현재마력 {}", current_mp.value())},
            [current_mp](auto& resp) -> bool {
                return resp.ch_mp == current_mp.value();
            },
            timeout);
    }

    co_return true;
}

async::task<uint8_t> game_bot::learn_spell(const std::string& spell_name, std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<game_resp::spell_update>(
        game_reqs::chat{false, std::format("/마법배우기 {}", spell_name)},
        [](auto& resp) -> bool {
            return resp.index != 0xFF;
        },
        timeout);

    co_return resp.index;
}

async::task<size_t> game_bot::learn_spells(const std::vector<std::string>& spell_names,
                                           std::chrono::milliseconds       timeout)
{

    size_t learned_count = 0;
    for (const auto& spell_name : spell_names)
    {
        auto index = co_await this->learn_spell(spell_name, timeout);
        this->chat(std::format("Learned spell: {} at index: {}", spell_name, index));
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

    auto count = this->spells().size();
    if (count == 0)
        co_return;

    auto last_slot = uint8_t{0};
    for (auto& [slot, spell] : this->spells())
    {
        last_slot = std::max<uint8_t>(last_slot, slot);
    }

    this->send(game_reqs::chat{false, "/마법지우기"});
    co_return; // Command should succeed if bot is valid
}

async::task<void> game_bot::clear_all_drop_items(std::chrono::milliseconds timeout)
{

    this->send(game_reqs::chat{false, "/아이템삭제"});
    co_return;
}

async::task<void> game_bot::clear_inventory(std::chrono::milliseconds timeout)
{
    if (this->_items.size() == 0)
        co_return;

    auto last_slot = uint8_t{0};
    for (auto& [slot, item] : this->_items)
    {
        last_slot = std::max<uint8_t>(last_slot, slot);
    }

    std::ignore = co_await this->request<game_resp::item_remove>(
        game_reqs::chat{false, "/아이템초기화"},
        [last_slot](auto& resp) -> bool {
            return resp.index == last_slot;
        },
        timeout);
    co_return;
}

async::task<void> game_bot::fill_inventory(const std::string& name, std::chrono::milliseconds timeout)
{
    constexpr auto CONTAINER_CAPACITY = 52;

    // Clear inventory first
    co_await this->clear_inventory(timeout);

    // Fill inventory with the specified item
    for (int i = 0; i < CONTAINER_CAPACITY; ++i)
    {
        co_await this->create_item(name, 1, timeout);

        // Calculate and display progress percentage
        auto progress = static_cast<int>((i + 1) * 100.0 / CONTAINER_CAPACITY);
        this->chat(std::format("Fill inventory progress: {}%", progress));
    }

    co_return;
}

async::task<void> game_bot::move_bot_back_to_position(const fb::model::point<uint16_t>& original_position,
                                                      std::chrono::milliseconds         interval,
                                                      std::chrono::milliseconds         timeout)
{
    auto thread           = this->thread();
    auto current_position = this->position();
    auto move_y_axis      = current_position.y - original_position.y;

    if (move_y_axis > 0)
    {
        for (auto i = 0; i < move_y_axis; i++)
        {
            this->send(game_reqs::move{DIRECTION::TOP, this->oid(), current_position});
            co_await thread->sleep(interval);
            current_position.y--;
            this->set_position(current_position);
        }

        co_await this->direction(DIRECTION::BOTTOM, timeout);
    }
}

async::task<void> game_bot::reverse_condition(const std::vector<fb::model::dsl>& conditions,
                                              std::chrono::milliseconds          timeout)
{
    for (auto& condition : conditions)
    {
        switch (condition.header)
        {
        case fb::model::enum_value::DSL::gender:
        {
            auto params = dsl::gender(condition.params);
            if (params.value == GENDER::MAN)
                co_await this->change_sex(GENDER::WOMAN, timeout);
            else
                co_await this->change_sex(GENDER::MAN, timeout);
        }
        break;

        case fb::model::enum_value::DSL::level:
        {
            auto params = dsl::level(condition.params);
            co_await this->change_level(1, timeout);
        }
        break;

        case fb::model::enum_value::DSL::strength:
        case fb::model::enum_value::DSL::intelligence:
        case fb::model::enum_value::DSL::dexterity:
        {
            auto params = dsl::strength(condition.params);
            co_await this->change_stats(0, 0, 0, timeout);
        }
        break;

        case fb::model::enum_value::DSL::class_t:
        case fb::model::enum_value::DSL::promotion:
        {
            co_await this->change_class("평민", timeout);
        }
        break;

        default:
        {
            fb::logger::warn("Unknown condition: {}", enum_tostring<fb::model::enum_value::DSL>(condition.header));
        }
        break;
        }
    }
}

async::task<void> game_bot::apply_condition(const std::vector<fb::model::dsl>& conditions,
                                            std::chrono::milliseconds          timeout)
{
    static auto class_names = std::unordered_map<CLASS, std::unordered_map<uint8_t, std::string>>{
        {CLASS::WARRIOR, {{0, "전사"}, {1, "검객"}, {2, "검제"}, {3, "검황"}, {4, "검성"}}  },
        {CLASS::MAGE,    {{0, "주술사"}, {1, "술사"}, {2, "현사"}, {3, "현인"}, {4, "현자"}}},
        {CLASS::ROGUE,   {{0, "도적"}, {1, "자객"}, {2, "진검"}, {3, "귀검"}, {4, "태성"}}  },
        {CLASS::POET,    {{0, "도사"}, {1, "도인"}, {2, "명인"}, {3, "진인"}, {4, "진선"}}  }
    };

    auto required_class     = CLASS::NONE;
    auto required_promotion = 0;

    for (auto& condition : conditions)
    {
        switch (condition.header)
        {
        case fb::model::enum_value::DSL::gender:
        {
            auto params = dsl::gender(condition.params);
            co_await this->change_sex(params.value, timeout);
        }
        break;

        case fb::model::enum_value::DSL::level:
        {
            auto params = dsl::level(condition.params);
            co_await this->change_level(params.min.value(), timeout);
        }
        break;

        case fb::model::enum_value::DSL::strength:
        {
            auto params = dsl::strength(condition.params);
            co_await this->change_str(params.value, timeout);
        }
        break;

        case fb::model::enum_value::DSL::intelligence:
        {
            auto params = dsl::intelligence(condition.params);
            co_await this->change_int(params.value, timeout);
        }
        break;

        case fb::model::enum_value::DSL::dexterity:
        {
            auto params = dsl::dexterity(condition.params);
            co_await this->change_dex(params.value, timeout);
        }
        break;

        case fb::model::enum_value::DSL::class_t:
        {
            auto params    = dsl::class_t(condition.params);
            required_class = params.value;
        }
        break;

        case fb::model::enum_value::DSL::promotion:
        {
            auto params        = dsl::promotion(condition.params);
            required_promotion = params.value;
        }
        break;

        default:
        {
            fb::logger::warn("Unknown condition: {}", enum_tostring<fb::model::enum_value::DSL>(condition.header));
        }
        break;
        }
    }

    if (required_class != CLASS::NONE)
    {
        auto class_name = class_names[required_class][required_promotion];
        co_await this->change_class(class_name, timeout);
    }
}

async::task<void> game_bot::update_internal_info(std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<game_resp::internal_info>(
        game_reqs::self_info{},
        [](auto& resp) -> bool {
            return true;
        },
        timeout);
}

async::task<bool> game_bot::invite_group(std::shared_ptr<game_bot> target, std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<game_resp::message>(
        game_reqs::group{target->name()},
        [](auto& resp) -> bool {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            if (resp.text == _TEXT(MESSAGE_GROUP_JOINED_SUCCESS))
                return false;

            return true;
        },
        timeout);

    co_return resp.text == std::format(_TEXT(MESSAGE_GROUP_JOINED), target->name());
}

async::task<bool> game_bot::leave_group(std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<game_resp::message>(
        game_reqs::group{this->name()},
        [](auto& resp) -> bool {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_GROUP_LEFT_SUCCESS);
        },
        timeout);

    co_return true;
}

async::task<bool> game_bot::kick_group(std::shared_ptr<game_bot> target, std::chrono::milliseconds timeout)
{
    auto&& resp = co_await this->request<game_resp::message>(
        game_reqs::group{target->name()},
        [&target](auto& resp) -> bool {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == std::format(_TEXT(MESSAGE_GROUP_MEMBER_KICKED), target->name());
        },
        timeout);

    co_return true;
}

async::task<bool> game_bot::change_clan_role(std::shared_ptr<game_bot> target,
                                             CLAN_ROLE                 role,
                                             std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::menu && resp.look == npc->look;
        },
        timeout);

    if (resp.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::list;
        },
        timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to role change option (menu item 5)
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 5, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::input;
        },
        timeout);

    if (resp3.message != "상대 이름 입력")
    {
        co_return false;
    }

    // Enter target name
    auto&& resp4 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::INPUT, 0, target->name(), 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_type::input;
        },
        timeout);

    if (resp4.message != "직책 입력")
    {
        co_return false;
    }

    // Enter role value
    auto&& resp5 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::INPUT,
                          0,
                          std::to_string(static_cast<uint8_t>(role)),
                          0,
                          0,
                          "",
                          DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            return resp.type == fb::bot::integration::dialog_ext_type::normal;
        },
        timeout);

    if (resp5.message != "직책 변경 성공")
    {
        co_return false;
    }

    co_return true;
}

async::task<bool> game_bot::invite_to_clan(std::shared_ptr<game_bot> invitee, std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp1 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        timeout);

    co_await this->update_internal_info(timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to invite target selection dialog
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 2, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::input)
                return false;

            return true;
        },
        timeout);

    if (resp3.message != "상대 이름 입력")
    {
        co_return false;
    }

    // Send invite to target
    auto&& resp4 = co_await this->request<fb::bot::integration::dialog_bot>(
        invitee,
        game_reqs::dialog(game_reqs::dialog::INTERACTION::INPUT, 0x02, invitee->name(), 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp4.message != std::format("{} 문파에 가입?", this->clan_name()))
    {
        co_return false;
    }

    // Target accepts clan invite
    auto&& resp5 = co_await invitee->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp5.message != std::format("{} 문파에 가입됨", this->clan_name()))
    {
        co_return false;
    }

    // Target confirms clan join completion
    auto&& resp6 = co_await invitee->request<fb::bot::integration::dialog_ext_bot>(
        this->shared_from_this_as<game_bot>(),
        game_reqs::dialog(game_reqs::dialog::INTERACTION::NORMAL, 1, "", 0, 0, "", DIALOG_RESULT::QUIT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp6.message != std::format("{}가 승락함", invitee->name()))
    {
        co_return false;
    }

    // Clan master confirms invite completion
    this->send(game_reqs::dialog(game_reqs::dialog::INTERACTION::NORMAL, 1, "", 0, 0, "", DIALOG_RESULT::QUIT));

    // Verify clan invite result by checking target's clan info
    auto&& resp = co_await this->request<game_resp::external_info>(
        game_reqs::click(invitee->oid()),
        [oid = invitee->oid()](auto& resp) {
            return resp.oid == oid;
        },
        timeout);

    if (resp.clan_name != this->clan_name())
    {
        co_return false;
    }

    auto clan_title = std::format("{}타이틀", resp.clan_name);
    if (resp.clan_title != clan_title)
    {
        co_return false;
    }

    co_return true;
}

async::task<bool> game_bot::kick_from_clan(std::shared_ptr<game_bot> target, std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp1 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        timeout);

    co_await this->update_internal_info(timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to kick target selection dialog (menu item 4)
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 4, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::input)
                return false;

            return true;
        },
        timeout);

    if (resp3.message != "상대 이름 입력")
    {
        co_return false;
    }

    // Enter target name
    auto&& resp4 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::INPUT, 0, target->name(), 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp4.message != "추방했음")
    {
        co_return false;
    }

    co_return true;
}

async::task<bool> game_bot::leave_clan(std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp1 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        timeout);

    co_await this->update_internal_info(timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to leave clan option (menu item 3)
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 3, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp3.message != "클랜 탈퇴 성공")
    {
        co_return false;
    }

    co_return true;
}

async::task<bool> game_bot::destroy_clan(std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp1 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        timeout);

    co_await this->update_internal_info(timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to destroy clan option (menu item 1)
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 1, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp3.message != "클랜 제거 성공")
    {
        co_return false;
    }

    co_return true;
}

async::task<bool> game_bot::change_clan_title(const std::string& title, std::chrono::milliseconds timeout)
{
    // Find NPC 낙랑 for clan management
    auto npc = table::npc.name2npc("낙랑");
    if (npc == nullptr)
    {
        co_return false;
    }

    // Open NPC dialog
    auto&& resp1 = co_await this->request<fb::bot::integration::dialog_bot>(
        game_reqs::click(1),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_type::menu)
                return false;

            return true;
        },
        timeout);

    if (resp1.message != "안녕하세요. 무엇을 도와드릴까요?")
    {
        co_return false;
    }

    // Select clan management menu
    auto&& resp2 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::MENU, 0, "", 1, 0, "", DIALOG_RESULT::PREV),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::list)
                return false;

            return true;
        },
        timeout);

    co_await this->update_internal_info(timeout);

    if (resp2.message != std::format("클랜 이름 : {}", this->clan_name()))
    {
        co_return false;
    }

    // Navigate to title change option (menu item 0)
    auto&& resp3 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::LIST, 0, "", 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::input_ext)
                return false;

            return true;
        },
        timeout);

    if (resp3.message != "문파 칭호 입력")
    {
        co_return false;
    }

    // Enter new title
    auto&& resp4 = co_await this->request<fb::bot::integration::dialog_ext_bot>(
        game_reqs::dialog(game_reqs::dialog::INTERACTION::INPUT, 0, title, 0, 0, "", DIALOG_RESULT::NEXT),
        [&npc](auto& resp) {
            if (resp.look != npc->look)
                return false;

            if (resp.type != fb::bot::integration::dialog_ext_type::normal)
                return false;

            return true;
        },
        timeout);

    if (resp4.message != "문파 칭호 변경 성공")
    {
        co_return false;
    }

    co_await this->update_internal_info(timeout);
    co_return this->clan_title() == title;
}

// simple_item methods implementation
bool game_bot::simple_item::is_equipment(const game_bot_controller& controller) const
{
    // Equipment types used in tests (from item_test.equipment.cpp)
    static const auto equipment_types = std::unordered_set<ITEM_TYPE>{ITEM_TYPE::WEAPON,
                                                                      ITEM_TYPE::ARMOR,
                                                                      ITEM_TYPE::HELMET,
                                                                      ITEM_TYPE::RING,
                                                                      ITEM_TYPE::SHIELD,
                                                                      ITEM_TYPE::AUXILIARY};

    try
    {
        auto item_model = table::item.name2item(name);
        if (!item_model)
        {
            return false;
        }

        return equipment_types.contains(item_model->type);
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

std::string game_bot::simple_item::get_equipment_info(const game_bot_controller& controller) const
{
    try
    {
        auto item_model = table::item.name2item(name);
        if (!item_model)
        {
            return "";
        }

        // Check if it's equipment
        if (!is_equipment(controller))
        {
            return "";
        }

        std::stringstream sstream;
        auto&             model = static_cast<fb::model::equipment&>(*item_model);

        sstream << name << std::endl;

        // Add durability info if available (for equipment with durability)
        if (model.durability > 0)
        {
            sstream << "내구성: " << std::to_string(model.durability) << '/' << std::to_string(model.durability) << ' '
                    << std::fixed << std::setprecision(1) << 100.0 << '%' << std::endl;
        }

        // Add weapon damage info if it's a weapon
        if (item_model->type == ITEM_TYPE::WEAPON)
        {
            auto& weapon_model = static_cast<const fb::model::weapon&>(*item_model);
            sstream << "파괴력: 　　 S:　" << std::to_string(weapon_model.damage_small.min) << 'm'
                    << std::to_string(weapon_model.damage_small.max) << std::endl;
            sstream << "　　　  　 　L:　" << std::to_string(weapon_model.damage_large.min) << 'm'
                    << std::to_string(weapon_model.damage_large.max) << std::endl;
        }

        // Add basic stats
        sstream << "무장:   " << std::to_string(model.defensive_physical) << " Hit:  " << std::to_string(model.hit)
                << " Dam:  " << std::to_string(model.damage);

        // Add stat bonuses
        if (model.base_hp)
            sstream << std::left << std::setw(14) << std::endl << "체력치 상승:" << std::to_string(model.base_hp);

        if (model.base_mp)
            sstream << std::left << std::setw(14) << std::endl << "마력치 상승:" << std::to_string(model.base_mp);

        if (model.strength)
            sstream << std::left << std::setw(14) << std::endl << "힘 상승:" << std::to_string(model.strength);

        if (model.dexterity)
            sstream << std::left << std::setw(14) << std::endl << "민첩성 상승:" << std::to_string(model.dexterity);

        if (model.intelligence)
            sstream << std::left << std::setw(14) << std::endl << "지력 상승:" << std::to_string(model.intelligence);

        if (model.healing_cycle)
            sstream << std::left << std::setw(14) << std::endl << "재생력 상승:" << std::to_string(model.healing_cycle);

        // Add class and level requirements
        auto cls   = CLASS::NONE;
        auto level = uint8_t(0);
        for (auto& dsl : model.condition)
        {
            switch (dsl.header)
            {
            case DSL::class_t:
                cls = fb::model::dsl::class_t(dsl.params).value;
                break;

            case DSL::level:
                level = fb::model::dsl::level(dsl.params).min.value_or(0);
                break;
            }
        }

        sstream << std::endl;
        switch (cls)
        {
        case CLASS::NONE:
            sstream << "직업제한무";
            break;

        default:
            sstream << table::promotion[cls][0].name << "용";
            break;
        }

        sstream << " 레벨 " << std::to_string(level) << " 이상";

        // Add description if available
        if (model.desc.empty() == false)
            sstream << std::endl << std::endl << model.desc;

        return sstream.str();
    }
    catch (const std::exception& e)
    {
        return "";
    }
}

uint32_t game_bot::simple_item::get_price(const game_bot_controller& controller) const
{
    try
    {
        auto item_model = table::item.name2item(name);
        if (!item_model)
        {
            return 0xFFFFFFFF;
        }

        return item_model->price;
    }
    catch (const std::exception& e)
    {
        return 0xFFFFFFFF;
    }
}