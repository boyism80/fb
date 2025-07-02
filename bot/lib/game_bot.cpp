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
    this->pattern(&game_bot::pattern_pickup, 250ms, 1000ms);
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
const std::set<std::string>& game_bot::active_spells() const
{
    return this->_active_spells;
}

void game_bot::add_spell(const std::string& name)
{
    this->_active_spells.insert(name);
}

void game_bot::remove_spell(const std::string& name)
{
    this->_active_spells.erase(name);
}

bool game_bot::has_spell(const std::string& name) const
{
    return this->_active_spells.count(name) > 0;
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

uint8_t game_bot::sex() const
{
    return this->_sex;
}

void game_bot::set_sex(uint8_t value)
{
    this->_sex = value;
}

uint8_t game_bot::state() const
{
    return this->_state;
}

void game_bot::set_state(uint8_t value)
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
    this->send(fb::protocol::game::request::chat(false, message));
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

async::task<void> game_bot::pattern_pickup()
{
    this->send(fb::protocol::game::request::pick_up{false});
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