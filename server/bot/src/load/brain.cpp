#include <fb/bot/load/brain.h>

#include <fb/bot/game_controller.h>
#include <fb/encoding.h>
#include <fb/game/dialog_type.h>
#include <fb/logger.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <unordered_set>
#include <vector>

using namespace fb::bot::load;

namespace game_reqs = fb::protocol::game::request;

constexpr auto INTERRUPT_MARGIN    = 0.3;
constexpr auto COMMITMENT          = 0.2;
constexpr auto GOAL_COOLDOWN       = std::chrono::seconds(10);
constexpr auto SHOP_AVOID          = std::chrono::minutes(5);
constexpr auto ACTION_TIMEOUT      = std::chrono::milliseconds(1500);
constexpr auto APPRECIATE_HP       = 100u;
constexpr auto APPRECIATE_STEPS    = 80u;
constexpr auto CLEAR_KILL_STEPS    = 6.0;
constexpr auto UNPICKABLE_AVOID    = std::chrono::minutes(5);
constexpr auto WARMTH_AVOID        = std::chrono::minutes(30);
constexpr auto ATTACK_MISSES       = 3;
constexpr auto HIT_MEMORY          = std::chrono::seconds(60);
constexpr auto STUCK_TIMEOUT       = std::chrono::seconds(60);
constexpr auto GROUND_AVOID        = std::chrono::minutes(10);
constexpr auto TARGET_AVOID        = std::chrono::seconds(60);
constexpr auto HUNT_SLICE          = std::chrono::minutes(3);
constexpr auto HUNT_GRACE          = std::chrono::minutes(2);
constexpr auto FIGHT_TIMEOUT       = std::chrono::seconds(20);
constexpr auto REST_TIMEOUT        = std::chrono::seconds(90);
constexpr auto WARP_TIMEOUT        = std::chrono::seconds(5);
constexpr auto DIALOG_TIMEOUT      = std::chrono::seconds(3);
constexpr auto ITEM_LOOT_RANGE     = 6;
constexpr auto TALK_RANGE          = 3;
constexpr auto RETREAT_RANGE       = 8;
constexpr auto WALK_REPLAN_LIMIT   = 30;
constexpr auto BLOCK_MEMORY        = std::chrono::seconds(60);
constexpr auto WALK_STEPS_PER_PLAN = 20;
constexpr auto GROUND_STAY         = 600.0;
constexpr auto MATERIAL_BONUS      = 1.0;
constexpr auto ARMOR_ALPHA         = 0.1;
constexpr auto RETURN_MP           = 30u;
constexpr auto CARDINAL_SAVING     = 40.0;
constexpr auto WEAPON_UPGRADE      = 1.2;
constexpr auto ARMOR_UPGRADE       = 2.0;
constexpr auto ESCAPE_COOLDOWN     = std::chrono::minutes(5);
constexpr auto DPS_ALPHA           = 0.2;
constexpr auto MIN_DPS             = 0.5;
constexpr auto KILL_TIME_RATIO     = 0.6;
constexpr auto MONEY_NEED_WEIGHT   = 50.0;
constexpr auto SPELL_RETRY         = std::chrono::seconds(10);
constexpr auto MP_CHARGE_RETRY     = std::chrono::seconds(1);
constexpr auto MP_LOW_RATIO        = 0.3;
constexpr auto MP_CHARGE_COST      = 0.4;
constexpr auto MP_RESERVE_RATIO    = 0.2;
constexpr auto DRINK_WAIT          = "drink";
constexpr auto RECLAIM_WINDOW      = std::chrono::minutes(10);
constexpr auto RECLAIM_LOOTS       = 5;
constexpr auto MARKET_CLAIM        = std::chrono::minutes(5);
constexpr auto ADVERTISE_INTERVAL  = std::chrono::seconds(30);
constexpr auto TRADE_SEARCHES      = 3;
constexpr auto LEARN_WEIGHT_BASE   = 2.0;
constexpr auto LEARN_WEIGHT_CAP    = 1.5;
constexpr auto DEBUFF_RENEW_RATIO  = 0.8;
constexpr auto AMBUSH_RETRY        = std::chrono::seconds(3);
constexpr auto BUFF_CONFIRM        = std::chrono::seconds(2);
constexpr auto DEBUFF_SECONDS      = 10.0;
constexpr auto GROUP_SHUN          = std::chrono::minutes(10);
constexpr auto MISFIRE_LIMIT       = 5u;
constexpr auto EQUIP_SETTLE        = std::chrono::seconds(15);
constexpr auto QUEST_EXP_VALUE     = 0.5;
constexpr auto QUEST_SHUN          = std::chrono::minutes(30);
constexpr auto QUEST_RECHECK       = std::chrono::seconds(10);
constexpr auto QUEST_STEPS         = 1500u;
constexpr auto QUEST_HOPS          = 32u;
constexpr auto TRADE_BATCH         = 5u;
constexpr auto QUEST_TALKS         = 3u;
constexpr auto STATE_INTERVAL      = std::chrono::seconds(15);
constexpr auto TUTORIAL_GRACE      = std::chrono::seconds(10);
constexpr auto HURT_MEMORY         = std::chrono::minutes(30);
constexpr auto TRADE_COOLDOWN      = std::chrono::minutes(2);
constexpr auto QUEST_STOPS         = 1u;
constexpr auto BUFF_RENEW_RATIO    = 0.9;
constexpr auto LEADER_TIMEOUT      = std::chrono::minutes(3);
constexpr auto ANNOUNCE_INTERVAL   = std::chrono::seconds(30);
constexpr auto GROUP_RETRY         = std::chrono::minutes(2);
constexpr auto FOLLOW_RANGE        = 3;
constexpr auto SUPPORT_RATIO       = 0.5;
constexpr auto SUPPORT_IDLE        = std::chrono::milliseconds(500);
constexpr auto LOOT_YIELD          = std::chrono::seconds(10);
constexpr auto PILES_LIMIT         = 256u;
constexpr auto MIN_DELAY           = std::chrono::milliseconds(10);
constexpr auto MOVE_SETTLE         = std::chrono::milliseconds(200);
constexpr auto FAILED_STREAK_CAP   = 3u;
constexpr auto LOOT_RESYNC_WAIT    = std::chrono::milliseconds(500);
constexpr auto WARP_RETRIES        = 2u;
constexpr auto CLICK_WAIT          = std::chrono::seconds(1);
constexpr auto REST_IDLE           = std::chrono::milliseconds(1000);
constexpr auto YIELD_PRIOR_MINUTES = 1.0;
constexpr auto FUNDING_MIN         = 30u;
constexpr auto PROBE_INTERVAL      = std::chrono::seconds(10);
constexpr auto PROBE_TIMEOUT       = std::chrono::seconds(10);
constexpr auto LAG_SAMPLING        = 10u;
constexpr auto YIELD_MIN           = 0.05;
constexpr auto YIELD_MAX           = 1.5;
constexpr auto NAMES_LIMIT         = 512u;
constexpr auto LEADER_HEAL_PERCENT = 60u;
constexpr auto FINISHER_HP_RATIO   = 0.8;
constexpr auto FINISHER_HITS       = 3.0;
constexpr auto GROUND_CANDIDATES   = 5;
constexpr auto MIN_PREYS           = 10u;
constexpr auto FARM_IDLE           = std::chrono::seconds(20);
constexpr auto PRIOR_FIGHTS        = 3.0;
constexpr auto PRIOR_DEATH_SAFE    = 0.01;
constexpr auto PRIOR_DEATH_RISKY   = 0.5;
constexpr auto BORROWED_FIGHTS     = 3u;
constexpr auto EXPLORE_FIGHTS      = 30u;
constexpr auto EXPLORE_HP_RATIO    = 0.9;
constexpr auto REST_SECONDS_FULL   = 60.0;
constexpr auto APPROACH_SECONDS    = 3.0;
constexpr auto RECORD_ALPHA        = 0.2;
constexpr auto TUTORIAL_SLICE      = std::chrono::seconds(60);
constexpr auto INFO_INTERVAL       = std::chrono::seconds(10);
constexpr auto STEP_AVOID          = std::chrono::minutes(30);
constexpr auto STEP_ATTEMPTS       = 6;
constexpr auto DIALOG_LIMIT        = 16;
constexpr auto DIALOG_CLOSE_OID    = 0xFFFFFFFD;
constexpr auto ITEM_LOOK_MIN       = 0xBFFF;

persona::persona(const Json::Value& json) :
    name(json["name"].asString()),
    ratio(json.get("ratio", 1.0).asDouble()),
    hunt(json["weights"].get("hunt", 1.0).asDouble()),
    rest(json["weights"].get("rest", 1.0).asDouble()),
    tutorial(json["weights"].get("tutorial", 1.5).asDouble()),
    job(json["weights"].get("job", 1.4).asDouble()),
    sell(json["weights"].get("sell", 1.3).asDouble()),
    supply(json["weights"].get("supply", 1.2).asDouble()),
    learn(json["weights"].get("learn", 2.0).asDouble()),
    equip(json["weights"].get("equip", 1.5).asDouble()),
    group(json["weights"].get("group", 1.6).asDouble()),
    follow(json["weights"].get("follow", 2.2).asDouble()),
    reclaim(json["weights"].get("reclaim", 1.8).asDouble()),
    trade(json["weights"].get("trade", 1.4).asDouble()),
    quest(json["weights"].get("quest", 1.8).asDouble()),
    wander(json["weights"].get("wander", 0.05).asDouble()),
    jitter(json.get("jitter", 0.1).asDouble()),
    rest_below(json.get("rest_below", 0.35).asDouble()),
    rest_until(json.get("rest_until", 0.9).asDouble()),
    max_damage_ratio(json.get("max_damage_ratio", 0.2).asDouble()),
    max_threat_ratio(json.get("max_threat_ratio", 0.04).asDouble()),
    max_death_rate(json.get("max_death_rate", 0.05).asDouble()),
    max_loss_ratio(json.get("max_loss_ratio", 0.7).asDouble()),
    explore_rate(json.get("explore_rate", 0.1).asDouble()),
    abort_hp_ratio(json.get("abort_hp_ratio", 0.3).asDouble()),
    temperature(json.get("softmax_temperature", 0.3).asDouble()),
    money_weight(json.get("money_weight", 1.0).asDouble()),
    step_delay(json.get("step_delay_ms", 300).asUInt()),
    attack_delay(json.get("attack_delay_ms", 600).asUInt()),
    attack_wait(attack_delay)
{ }

void persona::accelerate(double factor)
{
    this->speed = std::max(1.0, factor);
    this->step_delay =
        std::max(MIN_DELAY, std::chrono::milliseconds(static_cast<int64_t>(this->step_delay.count() / this->speed)));
    this->attack_delay =
        std::max(MIN_DELAY, std::chrono::milliseconds(static_cast<int64_t>(this->attack_delay.count() / this->speed)));
}

tutorial::tutorial(const Json::Value& json) :
    giver_map(static_cast<uint16_t>(json["giver"].get("map", 0xFFFF).asUInt())),
    giver_npc(json["giver"].get("npc", 0).asUInt())
{
    for (auto& entry : json["steps"])
    {
        auto task = entry.get("task", "").asString();
        auto step = tutorial::step{fb::CP949(entry["item"].asString(), fb::PLATFORM::WINDOWS),
                                   TASK::NONE,
                                   fb::CP949(entry.get("target", "").asString(), fb::PLATFORM::WINDOWS),
                                   entry.get("count", 1).asUInt()};
        if (task == "use")
            step.task = TASK::USE;
        else if (task == "travel")
            step.task = TASK::TRAVEL;
        else if (task == "hunt")
            step.task = TASK::HUNT;

        this->steps.push_back(std::move(step));
    }
}

job::job(const Json::Value& json) :
    level(static_cast<uint8_t>(json.get("level", 5).asUInt())),
    group_level(static_cast<uint8_t>(json.get("group_level", 15).asUInt())),
    group_gap(static_cast<uint8_t>(json.get("group_gap", 8).asUInt())),
    group_apart(std::chrono::seconds(json.get("group_apart", 300).asUInt())),
    item(fb::CP949(json.get("item", "").asString(), fb::PLATFORM::WINDOWS)),
    count(json.get("count", 0).asUInt()),
    ground(fb::CP949(json.get("ground", "").asString(), fb::PLATFORM::WINDOWS))
{
    for (auto& name : json["npcs"].getMemberNames())
    {
        this->npcs.insert({enum_parse<CLASS>(name), json["npcs"][name].asUInt()});
    }

    for (auto& name : json["demand"].getMemberNames())
    {
        this->demand.push_back({enum_parse<CLASS>(name), json["demand"][name].asDouble()});
    }
}

supplies::supplies(const Json::Value& json) :
    sell_slots(static_cast<uint8_t>(json.get("sell_slots", 40).asUInt()))
{
    for (auto& entry : json["restocks"])
    {
        this->restocks.push_back({fb::CP949(entry["item"].asString(), fb::PLATFORM::WINDOWS),
                                  entry["count"].asUInt(),
                                  fb::CP949(entry.get("unless", "").asString(), fb::PLATFORM::WINDOWS),
                                  entry.get("caster", false).asBool()});
    }

    for (auto& heal : json["heals"])
    {
        this->heals.push_back({fb::CP949(heal["item"].asString(), fb::PLATFORM::WINDOWS), heal["hp"].asUInt()});
    }

    for (auto& drink : json["drinks"])
    {
        this->drinks.push_back({fb::CP949(drink["item"].asString(), fb::PLATFORM::WINDOWS), drink["mp"].asUInt()});
    }

    for (auto& keep : json["keeps"])
    {
        this->keeps.push_back(fb::CP949(keep.asString(), fb::PLATFORM::WINDOWS));
    }

    for (auto& name : json["stocks"].getMemberNames())
    {
        this->stocks.insert({fb::CP949(name, fb::PLATFORM::WINDOWS), json["stocks"][name].asUInt()});
    }
}

spellbook::spellbook(const Json::Value& catalog, const Json::Value& config) :
    lookahead(static_cast<uint8_t>(config.get("lookahead", 5).asUInt()))
{
    static const auto roles = std::unordered_map<std::string, ROLE>{
        {"HEAL",    ROLE::HEAL   },
        {"ATTACK",  ROLE::ATTACK },
        {"BUFF",    ROLE::BUFF   },
        {"AREA",    ROLE::AREA   },
        {"STRIKE",  ROLE::STRIKE },
        {"AMBUSH",  ROLE::AMBUSH },
        {"ENCHANT", ROLE::ENCHANT},
        {"DEBUFF",  ROLE::DEBUFF },
        {"CURE",    ROLE::CURE   },
        {"CHARGE",  ROLE::CHARGE },
        {"TRAVEL",  ROLE::TRAVEL },
        {"GOTO",    ROLE::GOTO   },
        {"ESCAPE",  ROLE::ESCAPE },
        {"SHOUT",   ROLE::SHOUT  },
    };

    for (auto& name : config["skips"])
    {
        this->skips.insert(fb::CP949(name.asString(), fb::PLATFORM::WINDOWS));
    }

    for (auto& name : catalog["classes"].getMemberNames())
    {
        auto& source = catalog["classes"][name];
        auto& guild  = this->guilds[enum_parse<CLASS>(name)];
        guild.npc    = source["npc"].asUInt();
        for (auto& entry : source["spells"])
        {
            auto lesson  = spellbook::lesson{};
            lesson.level = static_cast<uint8_t>(entry["level"].asUInt());
            lesson.money = entry.get("money", 0).asUInt();
            for (auto& spell : entry["names"])
            {
                lesson.names.push_back(fb::CP949(spell.asString(), fb::PLATFORM::WINDOWS));
            }

            for (auto& material : entry["materials"])
            {
                lesson.materials.push_back(
                    {fb::CP949(material["name"].asString(), fb::PLATFORM::WINDOWS), material["count"].asUInt()});
            }
            guild.lessons.push_back(std::move(lesson));
        }
    }

    for (auto& name : catalog["spells"].getMemberNames())
    {
        auto& source     = catalog["spells"][name];
        auto  effect     = spellbook::effect{};
        effect.id        = source["id"].asUInt();
        effect.type      = enum_parse<SPELL_TYPE>(source["type"].asString());
        effect.mp        = source.get("mp", 0).asUInt();
        effect.hp        = source.get("hp", 0).asUInt();
        effect.damage    = source.get("damage", 0).asUInt();
        effect.buff_time = source.get("buff_time", 0).asUInt();
        effect.self      = source.get("self", false).asBool();
        effect.exclusive = source.get("exclusive", false).asBool();
        effect.cure      = fb::CP949(source.get("cure", "").asString(), fb::PLATFORM::WINDOWS);

        auto trigger   = source.get("trigger", "").asString();
        effect.trigger = trigger == "EMERGENCY" ? TRIGGER::EMERGENCY
                         : trigger == "MANA"    ? TRIGGER::MANA
                                                : TRIGGER::ALWAYS;

        auto role   = source.get("role", "OTHER").asString();
        auto found  = roles.find(role);
        effect.role = found != roles.end() ? found->second : ROLE::OTHER;

        auto weight   = config["weights"].get(role, 0.0).asDouble();
        effect.weight = config["overrides"].get(name, weight).asDouble();

        this->effects.insert({fb::CP949(name, fb::PLATFORM::WINDOWS), effect});
    }
}

bool spellbook::wanted(const lesson& lesson) const
{
    return this->weight(lesson) > 0.0;
}

double spellbook::weight(const lesson& lesson) const
{
    auto result = 0.0;
    for (auto& name : lesson.names)
    {
        auto effect = this->find(name);
        if (effect != nullptr && this->skips.contains(name) == false)
            result = std::max(result, effect->weight);
    }
    return result;
}

std::vector<const spellbook::lesson*> spellbook::pending(CLASS cls, const game_bot& bot, uint8_t level) const
{
    auto result = std::vector<const lesson*>{};
    auto guild  = this->guilds.find(cls);
    if (guild == this->guilds.end())
        return result;

    auto learned = std::unordered_set<std::string>{};
    for (auto& [_, spell] : bot.spells())
    {
        learned.insert(spell.name);
    }

    for (auto& lesson : guild->second.lessons)
    {
        if (lesson.level > level || this->wanted(lesson) == false)
            continue;

        auto known = std::any_of(lesson.names.begin(), lesson.names.end(), [&learned](const auto& name) {
            return learned.contains(name);
        });
        if (known == false)
            result.push_back(&lesson);
    }

    std::stable_sort(result.begin(), result.end(), [this](auto lhs, auto rhs) {
        return this->weight(*lhs) > this->weight(*rhs);
    });
    return result;
}

const spellbook::effect* spellbook::find(const std::string& name) const
{
    auto found = this->effects.find(name);
    return found != this->effects.end() ? &found->second : nullptr;
}

questbook::questbook(const Json::Value& catalog, const Json::Value& config)
{
    auto skips = std::unordered_set<uint32_t>{};
    for (auto& id : config["skips"])
    {
        skips.insert(id.asUInt());
    }

    auto declines = std::vector<std::string>{};
    for (auto& word : config["declines"])
    {
        declines.push_back(fb::CP949(word.asString(), fb::PLATFORM::WINDOWS));
    }

    auto items = [](const Json::Value& source) {
        auto result = std::unordered_map<std::string, uint32_t>{};
        for (auto& name : source.getMemberNames())
        {
            result.insert({fb::CP949(name, fb::PLATFORM::WINDOWS), source[name].asUInt()});
        }
        return result;
    };

    for (auto& source : catalog["quests"])
    {
        auto entry = quest{};
        entry.id   = source["id"].asUInt();
        if (skips.contains(entry.id) || source["starts"].empty() || source["mobs"].empty() == false)
            continue;

        for (auto& npc : source["starts"])
        {
            entry.npcs.push_back(npc.asUInt());
        }
        for (auto& npc : source["npcs"])
        {
            if (std::find(entry.npcs.begin(), entry.npcs.end(), npc.asUInt()) == entry.npcs.end())
                entry.npcs.push_back(npc.asUInt());
        }

        entry.level        = static_cast<uint8_t>(source.get("level", 0).asUInt());
        entry.consumes     = items(source["consumes"]);
        entry.money        = source.get("money", 0).asUInt();
        entry.rewards      = items(source["rewards"]["items"]);
        entry.reward_money = source["rewards"].get("money", 0).asUInt();
        entry.reward_exp   = source["rewards"].get("exp", 0).asUInt();
        for (auto& pick : source["picks"])
        {
            auto text     = fb::CP949(pick.asString(), fb::PLATFORM::WINDOWS);
            auto declined = std::any_of(declines.begin(), declines.end(), [&text](const auto& word) {
                return text.find(word) != std::string::npos;
            });
            if (declined == false)
                entry.picks.push_back(text);
        }

        auto rewarded = entry.rewards.empty() == false || entry.reward_money > 0 || entry.reward_exp > 0;
        if (rewarded && entry.npcs.size() == 1)
            this->quests.push_back(std::move(entry));
    }
}

void census::update(const std::array<uint32_t, CLASS_COUNT>& population)
{
    auto lock         = std::lock_guard(this->_mutex);
    this->_population = population;
}

std::optional<CLASS> census::reserve(const std::string&                           name,
                                     const std::vector<std::pair<CLASS, double>>& demand,
                                     std::mt19937&                                random)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_reservations.find(name);
    if (found != this->_reservations.end())
        return found->second;

    if (demand.empty())
        return std::nullopt;

    auto counts = this->_population;
    for (auto& [_, value] : this->_reservations)
    {
        counts[static_cast<size_t>(value)]++;
    }

    auto total   = 1.0;
    auto weights = 0.0;
    for (auto& [value, weight] : demand)
    {
        total   += counts[static_cast<size_t>(value)];
        weights += weight;
    }

    auto deficits = std::vector<double>{};
    for (auto& [value, weight] : demand)
    {
        deficits.push_back(std::max(0.0, weight / weights - counts[static_cast<size_t>(value)] / total));
    }

    if (std::all_of(deficits.begin(), deficits.end(), [](auto deficit) {
            return deficit == 0.0;
        }))
    {
        deficits.clear();
        for (auto& [_, weight] : demand)
        {
            deficits.push_back(weight);
        }
    }

    auto index    = std::discrete_distribution<size_t>(deficits.begin(), deficits.end())(random);
    auto selected = demand.at(index).first;
    this->_reservations.insert({name, selected});

    auto event     = Json::Value{};
    event["kind"]  = "census";
    event["op"]    = "reserve";
    event["name"]  = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    event["class"] = enum_tostring(selected);
    this->_journal.record(std::move(event));
    return selected;
}

void census::release(const std::string& name)
{
    auto lock = std::lock_guard(this->_mutex);
    if (this->_reservations.erase(name) == 0)
        return;

    auto event    = Json::Value{};
    event["kind"] = "census";
    event["op"]   = "release";
    event["name"] = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    this->_journal.record(std::move(event));
}

fb::bot::cluster::journal& census::journal()
{
    return this->_journal;
}

void census::apply(const Json::Value& event)
{
    auto lock = std::lock_guard(this->_mutex);
    auto name = fb::CP949(event["name"].asString(), fb::PLATFORM::WINDOWS);
    if (event["op"].asString() == "reserve")
        this->_reservations.insert_or_assign(name, enum_parse<CLASS>(event["class"].asString()));
    else
        this->_reservations.erase(name);
}

bool hazards::add(uint16_t map, const point<uint16_t>& position)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto added = this->_tiles[map].insert((position.y << 16) | position.x).second;
    if (added)
    {
        auto event    = Json::Value{};
        event["kind"] = "hazard";
        event["map"]  = map;
        event["x"]    = position.x;
        event["y"]    = position.y;
        this->_journal.record(std::move(event));
    }
    return added;
}

fb::bot::cluster::journal& hazards::journal()
{
    return this->_journal;
}

void hazards::apply(const Json::Value& event)
{
    auto lock = std::lock_guard(this->_mutex);
    this->_tiles[static_cast<uint16_t>(event["map"].asUInt())].insert((event["y"].asUInt() << 16) |
                                                                      event["x"].asUInt());
}

std::unordered_set<uint32_t> hazards::tiles(uint16_t map) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_tiles.find(map);
    return found != this->_tiles.end() ? found->second : std::unordered_set<uint32_t>{};
}

void roster::update(const std::string& name, uint8_t level)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_levels.find(name);
    if (found != this->_levels.end() && found->second == level)
        return;

    this->_levels.insert_or_assign(name, level);

    auto event     = Json::Value{};
    event["kind"]  = "roster";
    event["op"]    = "update";
    event["name"]  = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    event["level"] = level;
    this->_journal.record(std::move(event));
}

void roster::remove(const std::string& name)
{
    auto lock = std::lock_guard(this->_mutex);
    if (this->_levels.erase(name) == 0)
        return;

    auto event    = Json::Value{};
    event["kind"] = "roster";
    event["op"]   = "remove";
    event["name"] = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    this->_journal.record(std::move(event));
}

fb::bot::cluster::journal& roster::journal()
{
    return this->_journal;
}

void roster::apply(const Json::Value& event)
{
    auto lock = std::lock_guard(this->_mutex);
    auto name = fb::CP949(event["name"].asString(), fb::PLATFORM::WINDOWS);
    if (event["op"].asString() == "update")
        this->_levels.insert_or_assign(name, static_cast<uint8_t>(event["level"].asUInt()));
    else
        this->_levels.erase(name);
}

std::optional<uint8_t> roster::level(const std::string& name) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_levels.find(name);
    return found != this->_levels.end() ? std::optional<uint8_t>{found->second} : std::nullopt;
}

bool market::post(const std::string&     name,
                  const std::string&     item,
                  uint32_t               count,
                  uint16_t               map,
                  const point<uint16_t>& position)
{
    auto lock    = std::lock_guard(this->_mutex);
    auto found   = std::find_if(this->_wants.begin(), this->_wants.end(), [&](const auto& want) {
        return want.name == name && want.item == item;
    });
    auto added   = found == this->_wants.end();
    auto changed = added || found->count != count || found->map != map;
    if (added)
    {
        this->_wants.push_back({name, item, count, map, position});
    }
    else
    {
        found->count    = count;
        found->map      = map;
        found->position = position;
    }

    if (changed)
    {
        auto event     = Json::Value{};
        event["kind"]  = "market";
        event["op"]    = "post";
        event["name"]  = fb::UTF8(name, fb::PLATFORM::WINDOWS);
        event["item"]  = fb::UTF8(item, fb::PLATFORM::WINDOWS);
        event["count"] = count;
        event["map"]   = map;
        event["x"]     = position.x;
        event["y"]     = position.y;
        this->_journal.record(std::move(event));
    }
    return added;
}

void market::withdraw(const std::string& name, const std::string& item)
{
    auto lock    = std::lock_guard(this->_mutex);
    auto removed = std::erase_if(this->_wants, [&](const auto& want) {
        return want.name == name && (item.empty() || want.item == item);
    });
    if (removed == 0)
        return;

    auto event    = Json::Value{};
    event["kind"] = "market";
    event["op"]   = "withdraw";
    event["name"] = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    event["item"] = fb::UTF8(item, fb::PLATFORM::WINDOWS);
    this->_journal.record(std::move(event));
}

void market::release(const std::string& giver)
{
    auto lock     = std::lock_guard(this->_mutex);
    auto released = false;
    for (auto& want : this->_wants)
    {
        if (want.giver == giver)
        {
            want.giver.clear();
            released = true;
        }
    }

    if (released == false)
        return;

    auto event     = Json::Value{};
    event["kind"]  = "market";
    event["op"]    = "release";
    event["giver"] = fb::UTF8(giver, fb::PLATFORM::WINDOWS);
    this->_journal.record(std::move(event));
}

bool market::wanted(const std::string& item, const std::string& except) const
{
    auto lock = std::lock_guard(this->_mutex);
    return std::any_of(this->_wants.begin(), this->_wants.end(), [&](const auto& want) {
        return want.item == item && want.name != except;
    });
}

std::optional<market::want> market::find(const std::string& name, const std::string& item) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = std::find_if(this->_wants.begin(), this->_wants.end(), [&](const auto& want) {
        return want.name == name && want.item == item;
    });
    return found != this->_wants.end() ? std::optional<want>{*found} : std::nullopt;
}

std::optional<market::want> market::peek(const std::string& giver, const std::function<bool(const want&)>& accept) const
{
    auto lock = std::lock_guard(this->_mutex);
    auto now  = std::chrono::steady_clock::now();
    for (auto& want : this->_wants)
    {
        auto open = want.giver.empty() || want.giver == giver || now >= want.claimed + MARKET_CLAIM;
        if (want.name != giver && open && accept(want))
            return want;
    }
    return std::nullopt;
}

std::optional<market::want> market::claim(const std::string& giver, const std::function<bool(const want&)>& accept)
{
    auto lock = std::lock_guard(this->_mutex);
    auto now  = std::chrono::steady_clock::now();
    for (auto& want : this->_wants)
    {
        auto open = want.giver.empty() || want.giver == giver || now >= want.claimed + MARKET_CLAIM;
        if (want.name != giver && open && accept(want))
        {
            want.giver   = giver;
            want.claimed = now;

            auto event     = Json::Value{};
            event["kind"]  = "market";
            event["op"]    = "claim";
            event["giver"] = fb::UTF8(giver, fb::PLATFORM::WINDOWS);
            event["name"]  = fb::UTF8(want.name, fb::PLATFORM::WINDOWS);
            event["item"]  = fb::UTF8(want.item, fb::PLATFORM::WINDOWS);
            this->_journal.record(std::move(event));
            return want;
        }
    }
    return std::nullopt;
}

fb::bot::cluster::journal& market::journal()
{
    return this->_journal;
}

void market::apply(const Json::Value& event)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto op    = event["op"].asString();
    auto name  = fb::CP949(event["name"].asString(), fb::PLATFORM::WINDOWS);
    auto item  = fb::CP949(event["item"].asString(), fb::PLATFORM::WINDOWS);
    auto giver = fb::CP949(event["giver"].asString(), fb::PLATFORM::WINDOWS);
    auto found = std::find_if(this->_wants.begin(), this->_wants.end(), [&](const auto& want) {
        return want.name == name && want.item == item;
    });

    if (op == "post")
    {
        auto position =
            point<uint16_t>{static_cast<uint16_t>(event["x"].asUInt()), static_cast<uint16_t>(event["y"].asUInt())};
        auto count = event["count"].asUInt();
        auto map   = static_cast<uint16_t>(event["map"].asUInt());
        if (found == this->_wants.end())
        {
            this->_wants.push_back({name, item, count, map, position});
        }
        else
        {
            found->count    = count;
            found->map      = map;
            found->position = position;
        }
    }
    else if (op == "withdraw")
    {
        std::erase_if(this->_wants, [&](const auto& want) {
            return want.name == name && (item.empty() || want.item == item);
        });
    }
    else if (op == "release")
    {
        for (auto& want : this->_wants)
        {
            if (want.giver == giver)
                want.giver.clear();
        }
    }
    else if (found != this->_wants.end())
    {
        found->giver   = giver;
        found->claimed = std::chrono::steady_clock::now();
    }
}

void experience::configure(const Json::Value& json)
{
    auto lock              = std::lock_guard(this->_mutex);
    this->_level_band      = static_cast<uint8_t>(std::max(1u, json.get("level_band", 10).asUInt()));
    this->_multiplier      = json.get("exp_multiplier", 1.0).asDouble();
    this->_drop_multiplier = json.get("drop_multiplier", 1.0).asDouble();
    for (auto& name : json["skills"].getMemberNames())
    {
        auto& skills = this->_skills[enum_parse<CLASS>(name)];
        for (auto& skill : json["skills"][name])
        {
            skills.push_back(fb::CP949(skill.asString(), fb::PLATFORM::WINDOWS));
        }
    }
}

double experience::multiplier() const
{
    auto lock = std::lock_guard(this->_mutex);
    return this->_multiplier;
}

double experience::drop_multiplier() const
{
    auto lock = std::lock_guard(this->_mutex);
    return this->_drop_multiplier;
}

std::vector<std::string> experience::keys(const game_bot& bot, CLASS cls) const
{
    auto lock   = std::lock_guard(this->_mutex);
    auto band   = bot.level() / this->_level_band;
    auto owned  = std::vector<std::string>{};
    auto skills = this->_skills.find(cls);
    if (skills != this->_skills.end())
    {
        for (auto& skill : skills->second)
        {
            auto learned = std::any_of(bot.spells().begin(), bot.spells().end(), [&skill](const auto& pair) {
                return pair.second.name == skill;
            });
            if (learned)
                owned.push_back(skill);
        }
    }

    auto joined = [](const std::vector<std::string>& values) {
        auto text = std::string{};
        for (auto& value : values)
        {
            text += value + ",";
        }
        return text;
    };

    auto group  = bot.group_info().empty() ? std::string{"S"} : std::string{"G"};
    auto other  = bot.group_info().empty() ? std::string{"G"} : std::string{"S"};
    auto prefix = std::format("{}|{}|", bot.class_name(), band);
    auto keys   = std::vector<std::string>{prefix + joined(owned) + "|" + group, prefix + joined(owned) + "|" + other};
    if (owned.empty() == false)
    {
        auto fewer = owned;
        fewer.pop_back();
        keys.push_back(prefix + joined(fewer) + "|" + group);
    }
    keys.push_back(prefix + "*");
    return keys;
}

void experience::add(const std::vector<std::string>& keys, const fight& value)
{
    auto lock = std::lock_guard(this->_mutex);
    this->_fights++;

    auto event       = Json::Value{};
    event["kind"]    = "fight";
    event["mob"]     = fb::UTF8(value.mob, fb::PLATFORM::WINDOWS);
    event["outcome"] = static_cast<uint32_t>(value.outcome);
    event["seconds"] = value.seconds;
    event["damage"]  = value.damage;
    event["loss"]    = value.loss;
    event["exp"]     = value.exp;
    for (auto& key : {keys.front(), keys.back()})
    {
        this->merge(key, value);
        event["keys"].append(fb::UTF8(key, fb::PLATFORM::WINDOWS));
    }
    this->_journal.record(std::move(event));
}

void experience::merge(const std::string& key, const fight& value)
{
    auto& record = this->_records[std::format("{}#{}", key, value.mob)];
    auto  alpha  = record.fights < 5 ? 1.0 / (record.fights + 1) : RECORD_ALPHA;
    record.fights++;
    if (value.outcome == OUTCOME::DIED)
        record.deaths++;
    else if (value.outcome == OUTCOME::ABORTED)
        record.aborts++;

    record.seconds += (value.seconds - record.seconds) * alpha;
    record.damage  += (value.damage - record.damage) * alpha;
    record.loss    += (value.loss - record.loss) * alpha;
    record.exp     += (value.exp - record.exp) * alpha;
}

fb::bot::cluster::journal& experience::journal()
{
    return this->_journal;
}

void experience::apply(const Json::Value& event)
{
    if (event.isMember("mob") == false)
        return;

    auto lock  = std::lock_guard(this->_mutex);
    auto value = fight{
        .mob     = fb::CP949(event["mob"].asString(), fb::PLATFORM::WINDOWS),
        .outcome = static_cast<OUTCOME>(event["outcome"].asUInt()),
        .seconds = event["seconds"].asDouble(),
        .damage  = event["damage"].asDouble(),
        .loss    = event["loss"].asDouble(),
        .exp     = event["exp"].asDouble(),
    };
    for (auto& key : event["keys"])
    {
        this->merge(fb::CP949(key.asString(), fb::PLATFORM::WINDOWS), value);
    }
}

std::optional<experience::record> experience::find(const std::vector<std::string>& keys, const std::string& mob) const
{
    auto lock = std::lock_guard(this->_mutex);
    for (size_t i = 0; i < keys.size(); i++)
    {
        auto found = this->_records.find(std::format("{}#{}", keys.at(i), mob));
        if (found == this->_records.end() || found->second.fights == 0)
            continue;

        auto record = found->second;
        if (i != 0 && record.fights > BORROWED_FIGHTS)
        {
            auto scale    = static_cast<double>(BORROWED_FIGHTS) / record.fights;
            record.deaths = static_cast<uint32_t>(std::lround(record.deaths * scale));
            record.aborts = static_cast<uint32_t>(std::lround(record.aborts * scale));
            record.fights = BORROWED_FIGHTS;
        }
        return record;
    }
    return std::nullopt;
}

uint64_t experience::fights() const
{
    auto lock = std::lock_guard(this->_mutex);
    return this->_fights;
}

brain::brain(const knowledge& knowledge,
             const persona&   persona,
             const tutorial&  tutorial,
             const job&       job,
             const supplies&  supplies,
             const spellbook& spellbook,
             const questbook& questbook,
             census&          census,
             hazards&         hazards,
             market&          market,
             roster&          roster,
             experience&      experience,
             const messages&  messages,
             trace&           trace,
             metrics&         metrics,
             uint32_t         seed) :
    _knowledge(knowledge),
    _persona(persona),
    _tutorial(tutorial),
    _job(job),
    _supplies(supplies),
    _spellbook(spellbook),
    _questbook(questbook),
    _census(census),
    _hazards(hazards),
    _market(market),
    _roster(roster),
    _experience(experience),
    _messages(messages),
    _trace(trace),
    _metrics(metrics),
    _random(seed)
{ }

async::task<void> brain::run(std::shared_ptr<game_bot> bot)
{
    auto thread = bot->thread();
    while (this->_running.exchange(true))
    {
        if (bot->inited() == false)
            co_return;

        co_await thread->sleep(100ms);
    }

    this->_owner         = bot->id;
    this->_interrupted   = false;
    this->_last_exp      = bot->exp();
    this->_was_ghost     = bot->state() == STATE::GHOST;
    this->_progress_time = clock::now();
    try
    {
        while (this->alive(*bot))
        {
            auto selected = GOAL::WANDER;
            auto best     = -1.0;
            auto scores   = Json::Value{Json::objectValue};
            for (size_t i = 0; i < GOAL_COUNT; i++)
            {
                auto goal  = static_cast<GOAL>(i);
                auto value = this->score(goal, *bot);
                if (value > 0.0)
                    value += std::uniform_real_distribution<double>(0.0, this->_persona.jitter)(this->_random);

                if (this->_goal == static_cast<int>(i))
                    value *= 1.0 + COMMITMENT;

                if (value > 0.0)
                    scores[GOAL_NAMES.at(i)] = std::round(value * 1000.0) / 1000.0;

                if (value > best)
                {
                    best     = value;
                    selected = goal;
                }
            }

            if (this->_goal != static_cast<int>(selected))
            {
                auto entry      = Json::Value{};
                entry["event"]  = "goal";
                entry["goal"]   = GOAL_NAMES.at(static_cast<size_t>(selected));
                entry["score"]  = best;
                entry["scores"] = scores;
                this->_trace.write(bot->name(), entry);
            }

            this->_goal        = static_cast<int>(selected);
            this->_interrupted = false;

            auto outcome = result::SUCCEEDED;
            auto started = clock::now();
            switch (selected)
            {
            case GOAL::REVIVE:
                outcome = co_await this->revive(*bot);
                break;

            case GOAL::REST:
                outcome = co_await this->rest(*bot);
                break;

            case GOAL::TUTORIAL:
                outcome = co_await this->learn(*bot);
                break;

            case GOAL::JOB:
                outcome = co_await this->join(*bot);
                break;

            case GOAL::SELL:
                outcome = co_await this->sell(*bot);
                break;

            case GOAL::SUPPLY:
                outcome = co_await this->supply(*bot);
                break;

            case GOAL::HUNT:
                outcome = co_await this->hunt(*bot);
                break;

            case GOAL::WANDER:
                outcome = co_await this->wander(*bot);
                break;

            case GOAL::LEARN:
                outcome = co_await this->study(*bot);
                break;

            case GOAL::EQUIP:
                outcome = co_await this->equip(*bot);
                break;

            case GOAL::GROUP:
                outcome = co_await this->group(*bot);
                break;

            case GOAL::FOLLOW:
                outcome = co_await this->follow(*bot);
                break;

            case GOAL::RECLAIM:
                outcome = co_await this->reclaim(*bot);
                break;

            case GOAL::TRADE:
                outcome = co_await this->trade(*bot);
                break;

            case GOAL::QUEST:
                outcome = co_await this->quest(*bot);
                break;
            }

            if (outcome != result::INTERRUPTED)
                bot->controller.container.stats.add(
                    std::format("goal:{}", GOAL_NAMES.at(static_cast<size_t>(selected))),
                    std::chrono::duration<double, std::milli>(clock::now() - started).count(),
                    outcome == result::FAILED);

            if (outcome == result::SUCCEEDED)
                this->_failed_streak[static_cast<size_t>(selected)] = 0;

            if (outcome == result::FAILED)
            {
                auto streak = std::min(this->_failed_streak[static_cast<size_t>(selected)]++, FAILED_STREAK_CAP);
                this->_cooldowns[static_cast<size_t>(selected)] =
                    std::max(this->_cooldowns[static_cast<size_t>(selected)],
                             clock::now() + this->paced(GOAL_COOLDOWN) * (1u << streak));
                this->_metrics.failures[static_cast<size_t>(selected)]++;
                co_await thread->sleep(this->paced(500ms));
            }

            auto entry      = Json::Value{};
            entry["event"]  = "result";
            entry["goal"]   = GOAL_NAMES.at(static_cast<size_t>(selected));
            entry["result"] = outcome == result::SUCCEEDED ? "SUCCEEDED"
                              : outcome == result::FAILED  ? "FAILED"
                                                           : "INTERRUPTED";
            entry["map"]    = bot->map();
            entry["x"]      = bot->position().x;
            entry["y"]      = bot->position().y;
            this->_trace.write(bot->name(), entry);
        }
    }
    catch (std::exception& e)
    {
        fb::logger::warn("load brain: bot={} stopped: {}", bot->name(), e.what());
    }

    this->_goal    = -1;
    this->_running = false;
}

void brain::perceive(game_bot& bot)
{
    for (auto& event : bot.events.take())
    {
        if (event.type == game_bot::events::TYPE::HIT)
        {
            this->_hits.insert_or_assign(event.oid, std::pair{clock::now(), event.value});
            this->_vitals.insert_or_assign(event.oid, event.extra);
            if (event.oid == this->_fighting)
                this->_fight_damage += event.value;
            if (event.oid == bot.oid() && this->_opponent_damage > 0)
            {
                auto ratio   = std::clamp(static_cast<double>(event.value) / this->_opponent_damage, 0.05, 2.0);
                this->_armor = this->_armor + (ratio - this->_armor) * ARMOR_ALPHA;
            }
            continue;
        }

        if (event.type == game_bot::events::TYPE::MESSAGE && this->_clicked != 0 &&
            event.value == static_cast<uint32_t>(MESSAGE_TYPE::STATE))
        {
            if (this->_names.size() >= NAMES_LIMIT)
                std::erase_if(this->_names, [&bot](const auto& pair) {
                    return bot.sight.entries().contains(pair.first) == false;
                });
            this->_names.insert_or_assign(this->_clicked, event.text);
            this->_clicked = 0;
            continue;
        }

        auto text    = fb::UTF8(event.text, fb::PLATFORM::WINDOWS);
        auto speaker = std::string{};
        if (event.type == game_bot::events::TYPE::CHAT)
        {
            auto found = bot.sight.entries().find(event.oid);
            if (event.oid == bot.oid())
                speaker = "self";
            else if (found != bot.sight.entries().end() && found->second.name.empty() == false)
                speaker = fb::UTF8(found->second.name, fb::PLATFORM::WINDOWS);
            else
                speaker = std::format("npc:{}", event.oid);

            auto separator = text.find(event.value == static_cast<uint32_t>(CHAT_TYPE::SHOUT) ? "! " : ": ");
            if (separator != std::string::npos)
                text = text.substr(separator + 2);
        }

        auto kind      = this->_messages.classify(text);
        auto entry     = Json::Value{};
        entry["event"] = event.type == game_bot::events::TYPE::CHAT ? "chat" : "message";
        entry["kind"]  = kind.has_value() ? kind.value() : "UNKNOWN";
        entry["text"]  = text;
        if (speaker.empty() == false)
            entry["speaker"] = speaker;
        this->_trace.write(bot.name(), entry);

        if (kind.has_value() && kind->empty() == false)
            this->_heard.insert_or_assign(kind.value(), clock::now());

        if (kind == "GROUP_JOINED")
        {
            this->_group_ready = true;
        }
        else if (kind == "GROUP_LEFT")
        {
            this->_grouped    = false;
            this->_leader_map = 0xFFFF;
            this->_leader.clear();
            this->_partner.clear();
        }
        else if (event.type == game_bot::events::TYPE::MESSAGE && event.text.starts_with("<!"))
        {
            auto open  = event.text.find('(');
            auto close = event.text.find("> ");
            auto words = std::vector<std::string>{};
            if (open != std::string::npos && close != std::string::npos)
            {
                auto stream = std::istringstream(event.text.substr(close + 2));
                for (auto word = std::string{}; stream >> word;)
                {
                    words.push_back(word);
                }
            }

            auto sender = open != std::string::npos ? event.text.substr(2, open - 2) : std::string{};
            auto id     = uint16_t{0};
            auto x      = uint16_t{0};
            auto y      = uint16_t{0};
            auto parsed = words.size() == 5 && words[0] == "위치" &&
                          std::from_chars(words[2].data(), words[2].data() + words[2].size(), id).ec == std::errc{} &&
                          std::from_chars(words[3].data(), words[3].data() + words[3].size(), x).ec == std::errc{} &&
                          std::from_chars(words[4].data(), words[4].data() + words[4].size(), y).ec == std::errc{} &&
                          table::map->find(id) != nullptr;
            if (parsed && sender != bot.name())
            {
                if (this->_leader != sender)
                    this->_partner_seen = clock::now();
                this->_grouped         = true;
                this->_leader          = sender;
                this->_leader_map      = id;
                this->_leader_position = point<uint16_t>(x, y);
                this->_leader_seen     = clock::now();
            }
        }
    }
}

bool brain::heard(const std::string& kind, time_point since) const
{
    auto found = this->_heard.find(kind);
    return found != this->_heard.end() && found->second >= since;
}

async::task<std::string> brain::expect(game_bot&                       bot,
                                       time_point                      since,
                                       const std::vector<std::string>& failures,
                                       const std::function<bool()>&    done,
                                       std::chrono::milliseconds       timeout)
{
    auto deadline = clock::now() + timeout;
    while (this->alive(bot))
    {
        this->perceive(bot);
        if (done())
        {
            bot.controller.container.stats.add("rtt:expect",
                                               std::chrono::duration<double, std::milli>(clock::now() - since).count(),
                                               false);
            co_return std::string{};
        }

        for (auto& failure : failures)
        {
            if (this->heard(failure, since))
            {
                bot.controller.container.stats.add(
                    "rtt:expect",
                    std::chrono::duration<double, std::milli>(clock::now() - since).count(),
                    false);
                co_return failure;
            }
        }

        if (clock::now() >= deadline)
        {
            bot.controller.container.stats.add("rtt:expect",
                                               std::chrono::duration<double, std::milli>(clock::now() - since).count(),
                                               true);
            co_return std::string{"TIMEOUT"};
        }

        auto slept = clock::now();
        co_await bot.thread()->sleep(100ms);
        if (++this->_lag_samples % LAG_SAMPLING == 0)
            bot.controller.container.stats.add(
                "lag:loop",
                std::chrono::duration<double, std::milli>(clock::now() - slept - 100ms).count(),
                false);
    }
    co_return std::string{"INTERRUPTED"};
}

void brain::report(game_bot& bot, const char* action, const std::string& reason, Json::Value detail)
{
    detail["event"]  = "action";
    detail["action"] = action;
    detail["result"] = reason.empty() ? "SUCCEEDED" : "FAILED";
    if (reason.empty() == false)
        detail["reason"] = reason;
    detail["hp"]  = bot.hp();
    detail["map"] = bot.map();
    detail["x"]   = bot.position().x;
    detail["y"]   = bot.position().y;
    this->_trace.write(bot.name(), detail);
    bot.controller.container.stats.count(std::format("action:{}", action), reason.empty() == false);

    if (reason == "TIMEOUT" || reason == "NO_HIT")
        this->_misfires++;
    else if (reason.empty())
        this->_misfires = 0;
}

void brain::check(game_bot& bot)
{
    if (this->alive(bot) == false)
        return;

    this->perceive(bot);
    this->_roster.update(bot.name(), bot.level());
    if (clock::now() - this->_stated >= STATE_INTERVAL)
    {
        this->_stated = clock::now();

        auto current     = this->goal();
        auto entry       = Json::Value{};
        entry["event"]   = "state";
        entry["level"]   = bot.level();
        entry["exp"]     = bot.exp();
        entry["hp"]      = bot.hp();
        entry["max_hp"]  = std::max(bot.base_hp(), bot.hp());
        entry["mp"]      = bot.mp();
        entry["max_mp"]  = std::max(bot.base_mp(), bot.mp());
        entry["money"]   = bot.money();
        entry["map"]     = bot.map();
        entry["x"]       = bot.position().x;
        entry["y"]       = bot.position().y;
        entry["items"]   = static_cast<uint32_t>(bot.items().size());
        entry["ghost"]   = bot.state() == STATE::GHOST;
        entry["class"]   = fb::UTF8(bot.class_name(), fb::PLATFORM::WINDOWS);
        entry["goal"]    = current.has_value() ? GOAL_NAMES.at(static_cast<size_t>(current.value())) : "";
        entry["leader"]  = fb::UTF8(this->_leader, fb::PLATFORM::WINDOWS);
        entry["partner"] = fb::UTF8(this->_partner, fb::PLATFORM::WINDOWS);
        entry["quest"]   = this->_quest_active;
        if (this->_ground.has_value())
            entry["ground"] = this->_ground->map;
        entry["materials"] = Json::Value{Json::objectValue};
        for (auto& [item, count] : this->reserved(bot))
        {
            entry["materials"][fb::UTF8(item, fb::PLATFORM::WINDOWS)] =
                std::format("{}/{}", bot.get_item_count_by_name(item), count);
        }
        entry["spells"] = Json::Value{Json::arrayValue};
        for (auto& [_, spell] : bot.spells())
        {
            auto effect     = this->_spellbook.find(spell.name);
            auto known      = Json::Value{};
            known["name"]   = fb::UTF8(spell.name, fb::PLATFORM::WINDOWS);
            known["role"]   = effect != nullptr ? static_cast<int>(effect->role) : -1;
            known["weight"] = effect != nullptr ? effect->weight : 0.0;
            known["mp"]     = effect != nullptr ? effect->mp : 0u;
            entry["spells"].append(known);
        }
        this->_trace.write(bot.name(), entry);
    }

    if (this->_misfires >= MISFIRE_LIMIT)
    {
        this->_misfires = 0;
        bot.send(*bot.make<game_reqs::screen_refresh>());

        auto entry     = Json::Value{};
        entry["event"] = "resync";
        entry["map"]   = bot.map();
        entry["x"]     = bot.position().x;
        entry["y"]     = bot.position().y;
        this->_trace.write(bot.name(), entry);
    }
    this->announce(bot);
    this->advertise(bot);
    this->regroup(bot);

    auto stale = clock::now() - HIT_MEMORY;
    std::erase_if(this->_hits, [stale](const auto& pair) {
        return pair.second.first < stale;
    });

    auto exp = bot.exp();
    if (exp > this->_last_exp)
        this->_metrics.exp += exp - this->_last_exp;
    this->_last_exp = exp;

    auto ghost = bot.state() == STATE::GHOST;
    if (ghost && this->_was_ghost == false)
    {
        this->_metrics.deaths++;
        this->_remains_map      = bot.map();
        this->_remains_position = bot.position();
        this->_remains_until    = clock::now() + RECLAIM_WINDOW;
        this->_hurt_maps.insert_or_assign(bot.map(), clock::now() + HURT_MEMORY);
        this->_avoided_grounds.insert_or_assign(bot.map(), clock::now() + GROUND_AVOID);
        if (this->_ground.has_value() && this->_ground->map == bot.map())
            this->_ground.reset();
    }
    this->_was_ghost = ghost;

    auto top_hp = std::max(bot.base_hp(), bot.hp());
    if (ghost == false && top_hp > 0 && bot.map() == this->_sampled_map && bot.hp() < this->_sampled_hp &&
        bot.hp() < top_hp * this->_persona.rest_below &&
        (this->_ground.has_value() == false || this->_ground->map != bot.map()))
        this->_hurt_maps.insert_or_assign(bot.map(), clock::now() + HURT_MEMORY);
    this->_sampled_map = bot.map();
    this->_sampled_hp  = bot.hp();

    auto now = clock::now();
    if (bot.map() != this->_progress_map || bot.position() != this->_progress_position || exp != this->_progress_exp)
    {
        this->_progress_map      = bot.map();
        this->_progress_position = bot.position();
        this->_progress_exp      = exp;
        this->_progress_time     = now;
    }

    auto known = this->_knowledge.find_class(bot.class_name());
    if (known.has_value())
    {
        this->_class = static_cast<int>(known.value());
    }
    else if (now - this->_info_time > INFO_INTERVAL)
    {
        this->_info_time = now;
        bot.send(*bot.make<game_reqs::self_info>());
    }

    if (this->_probe_pending && bot.info_received() > this->_probe_sent)
    {
        this->_probe_pending = false;
        bot.controller.container.stats.add(
            "rtt:info",
            std::chrono::duration<double, std::milli>(bot.info_received() - this->_probe_sent).count(),
            false);
    }
    else if (this->_probe_pending && now - this->_probe_sent > PROBE_TIMEOUT)
    {
        this->_probe_pending = false;
        bot.controller.container.stats.add("rtt:info",
                                           std::chrono::duration<double, std::milli>(now - this->_probe_sent).count(),
                                           true);
    }
    else if (this->_probe_pending == false && now - this->_probe_sent > PROBE_INTERVAL)
    {
        this->_probe_sent    = now;
        this->_probe_pending = true;
        bot.send(*bot.make<game_reqs::self_info>());
    }

    auto current = this->goal();
    if (current.has_value() == false)
        return;

    if (current.value() != GOAL::REST && now - this->_progress_time > STUCK_TIMEOUT)
    {
        this->_metrics.stuck++;
        this->_cooldowns[static_cast<size_t>(current.value())] = now + this->paced(GOAL_COOLDOWN);
        this->_progress_time                                   = now;
        this->_interrupted                                     = true;

        auto entry     = Json::Value{};
        entry["event"] = "stuck";
        entry["goal"]  = GOAL_NAMES.at(static_cast<size_t>(current.value()));
        entry["map"]   = bot.map();
        entry["x"]     = bot.position().x;
        entry["y"]     = bot.position().y;
        this->_trace.write(bot.name(), entry);
        fb::logger::warn("load brain: bot={} stuck goal={} map={} pos=({},{})",
                         bot.name(),
                         static_cast<int>(current.value()),
                         bot.map(),
                         bot.position().x,
                         bot.position().y);
        return;
    }

    if (this->_interrupted)
        return;

    auto current_score = this->score(current.value(), bot);
    for (size_t i = 0; i < GOAL_COUNT; i++)
    {
        auto challenger = this->score(static_cast<GOAL>(i), bot);
        if (challenger > current_score + INTERRUPT_MARGIN)
        {
            this->_interrupted = true;

            auto entry     = Json::Value{};
            entry["event"] = "interrupt";
            entry["from"]  = GOAL_NAMES.at(static_cast<size_t>(current.value()));
            entry["to"]    = GOAL_NAMES.at(i);
            entry["score"] = current_score;
            entry["by"]    = challenger;
            this->_trace.write(bot.name(), entry);
            break;
        }
    }
}

std::optional<GOAL> brain::goal() const
{
    auto value = this->_goal.load();
    if (value < 0)
        return std::nullopt;

    return static_cast<GOAL>(value);
}

std::optional<CLASS> brain::cls() const
{
    auto value = this->_class.load();
    if (value < 0)
        return std::nullopt;

    return static_cast<CLASS>(value);
}

bool brain::alive(const game_bot& bot) const
{
    return bot.inited() && this->_owner == bot.id;
}

double brain::score(GOAL goal, const game_bot& bot) const
{
    if (clock::now() < this->_cooldowns[static_cast<size_t>(goal)])
        return 0.0;

    auto ghost    = bot.state() == STATE::GHOST;
    auto max_hp   = std::max(bot.base_hp(), bot.hp());
    auto hp_ratio = max_hp > 0 ? static_cast<double>(bot.hp()) / max_hp : 1.0;
    switch (goal)
    {
    case GOAL::REVIVE:
        return ghost ? 10.0 : 0.0;

    case GOAL::REST:
        if (ghost ||
            (bot.hp() > APPRECIATE_HP &&
             std::none_of(this->_supplies.heals.begin(), this->_supplies.heals.end(), [&bot](const auto& heal) {
                 return bot.has_item_by_name(heal.first);
             })))
            return 0.0;
        else if (this->_goal == static_cast<int>(GOAL::REST))
            return hp_ratio < this->_persona.rest_until ? this->_persona.rest * 2.0 : 0.0;
        else
            return hp_ratio < this->_persona.rest_below
                       ? this->_persona.rest * (2.0 + this->_persona.rest_below - hp_ratio)
                       : 0.0;

    case GOAL::TUTORIAL:
        return ghost == false && (this->find_step(bot) != nullptr || clock::now() < this->_tutorial_grace)
                   ? this->_persona.tutorial
                   : 0.0;

    case GOAL::JOB:
        return ghost == false && bot.level() >= this->_job.level &&
                       this->_knowledge.find_class(bot.class_name()) == CLASS::NONE
                   ? this->_persona.job
                   : 0.0;

    case GOAL::SELL:
        return ghost == false &&
                       (this->_sell_urgent || bot.items().size() >= this->_supplies.sell_slots ||
                        this->overstocked(bot) || this->underfunded(bot)) &&
                       this->sellable(bot).empty() == false
                   ? this->_persona.sell
                   : 0.0;

    case GOAL::SUPPLY:
        if (ghost || bot.money() == 0 || this->shortage(bot) == nullptr)
            return 0.0;
        else if (hp_ratio < this->_persona.rest_below)
            return this->_persona.rest * 2.0;
        else
            return this->_persona.supply;

    case GOAL::HUNT:
        return ghost || (this->_leader.empty() == false && this->leader_hunting()) ? 0.0 : this->_persona.hunt;

    case GOAL::WANDER:
        return this->_persona.wander;

    case GOAL::LEARN:
    {
        if (ghost)
            return 0.0;

        auto cls = this->_knowledge.find_class(bot.class_name());
        if (cls.has_value() == false || cls.value() == CLASS::NONE)
            return 0.0;

        auto lesson = this->affordable(bot);
        return lesson != nullptr ? this->_persona.learn *
                                       std::min(LEARN_WEIGHT_CAP, this->_spellbook.weight(*lesson) / LEARN_WEIGHT_BASE)
                                 : 0.0;
    }

    case GOAL::EQUIP:
        return ghost == false && this->upgrade(bot).has_value() ? this->_persona.equip : 0.0;

    case GOAL::GROUP:
    {
        auto cls = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
        if (ghost || cls == CLASS::NONE || bot.level() < this->_job.group_level || this->_grouped ||
            this->_leader.empty() == false)
            return 0.0;
        else if (this->_group_ready == false)
            return this->_persona.group;
        else if ((cls == CLASS::WARRIOR || cls == CLASS::ROGUE) && this->_grouped == false)
            return this->_persona.group;
        else
            return 0.0;
    }

    case GOAL::FOLLOW:
    {
        auto distance = std::abs(bot.position().x - this->_leader_position.x) +
                        std::abs(bot.position().y - this->_leader_position.y);
        auto distant = bot.map() != this->_leader_map || distance > FOLLOW_RANGE * 2;
        if (ghost || this->_leader.empty() || this->_leader_map == 0xFFFF ||
            bot.items().size() >= this->_supplies.sell_slots || this->leader_hunting() == false)
            return 0.0;
        else if (distant)
            return this->_persona.follow;
        else
            return this->_persona.follow * SUPPORT_RATIO;
    }

    case GOAL::RECLAIM:
        return ghost == false && this->_remains_map != 0xFFFF && clock::now() < this->_remains_until &&
                       hp_ratio >= this->_persona.rest_below
                   ? this->_persona.reclaim
                   : 0.0;

    case GOAL::QUEST:
    {
        if (ghost || this->_leader.empty() == false || hp_ratio < this->_persona.rest_below ||
            this->find_step(bot) != nullptr || this->_knowledge.find_class(bot.class_name()) == CLASS::NONE)
            return 0.0;

        auto quest = this->choose_quest(bot);
        if (quest == nullptr)
            return 0.0;

        auto ready = std::all_of(quest->consumes.begin(), quest->consumes.end(), [&](const auto& consume) {
            return bot.get_item_count_by_name(consume.first) >= consume.second ||
                   this->_knowledge.sellers(consume.first).empty() == false;
        });
        return ready ? this->_persona.quest : 0.0;
    }

    case GOAL::TRADE:
        return ghost == false && this->_leader.empty() && hp_ratio >= this->_persona.rest_below &&
                       this->_market
                           .peek(bot.name(),
                                 [this, &bot](const auto& want) {
                                     return this->surplus(bot, want, std::min(want.count, TRADE_BATCH));
                                 })
                           .has_value()
                   ? this->_persona.trade
                   : 0.0;
    }
    return 0.0;
}

bool brain::dangerous(const game_bot& bot, const knowledge::prey& prey, const std::vector<std::string>& keys) const
{
    auto dps =
        this->_dps >= 0.0 ? std::max(this->_dps, MIN_DPS) : static_cast<double>(std::max<uint8_t>(bot.level(), 1));
    auto slow =
        static_cast<double>(prey.hp) / dps > std::chrono::duration<double>(FIGHT_TIMEOUT).count() * KILL_TIME_RATIO;
    return slow || this->harmful(bot, prey, keys);
}

std::chrono::milliseconds brain::paced(std::chrono::milliseconds value) const
{
    return std::chrono::milliseconds(static_cast<int64_t>(value.count() / this->_persona.speed));
}

bool brain::harmful(const game_bot& bot, const knowledge::prey& prey, const std::vector<std::string>& keys) const
{
    auto max_hp = static_cast<double>(std::max(bot.base_hp(), bot.hp()));
    auto dps =
        this->_dps >= 0.0 ? std::max(this->_dps, MIN_DPS) : static_cast<double>(std::max<uint8_t>(bot.level(), 1));
    auto hit    = prey.damage * this->_armor;
    auto loss   = static_cast<double>(prey.hp) / dps / prey.interval * hit;
    auto risky  = hit > max_hp * this->_persona.max_damage_ratio || loss > max_hp * this->_persona.max_threat_ratio;
    auto prior  = risky ? PRIOR_DEATH_RISKY : PRIOR_DEATH_SAFE;
    auto record = this->_experience.find(keys, prey.name);
    if (record.has_value() == false)
        return risky;

    auto rate = (record->deaths + prior * PRIOR_FIGHTS) / (record->fights + PRIOR_FIGHTS);
    return rate > this->_persona.max_death_rate ||
           (record->fights >= PRIOR_FIGHTS && record->loss > this->_persona.max_loss_ratio);
}

const knowledge::prey* brain::find_prey(uint16_t map, uint16_t look, uint32_t oid) const
{
    auto named = this->_names.find(oid);
    for (auto& ground : this->_knowledge.hunting_grounds())
    {
        if (ground.map != map)
            continue;

        auto best = static_cast<const knowledge::prey*>(nullptr);
        for (auto& prey : ground.preys)
        {
            if (prey.look != look)
                continue;

            if (named != this->_names.end() && named->second == prey.name)
                return &prey;

            if (best == nullptr || prey.hp > best->hp)
                best = &prey;
        }
        return best;
    }
    return nullptr;
}

const knowledge::prey* brain::identify(game_bot&                        bot,
                                       const knowledge::hunting_ground& ground,
                                       const game_bot::sight::entry&    entry)
{
    auto matches = std::vector<const knowledge::prey*>{};
    for (auto& prey : ground.preys)
    {
        if (prey.look == entry.look)
            matches.push_back(&prey);
    }

    if (matches.size() <= 1)
        return matches.empty() ? nullptr : matches.front();

    auto named = this->_names.find(entry.oid);
    if (named != this->_names.end())
    {
        auto found = std::find_if(matches.begin(), matches.end(), [&named](const auto prey) {
            return prey->name == named->second;
        });
        if (found != matches.end())
            return *found;

        this->_names.erase(named);
    }

    auto now = clock::now();
    if (this->_clicked != entry.oid || now >= this->_clicked_at + CLICK_WAIT)
    {
        this->_clicked    = entry.oid;
        this->_clicked_at = now;
        bot.send(
            *bot.make<game_reqs::click>(entry.oid, game_reqs::click<fb::protocol::CLIENT_VERSION::v550>::FLAG_OBJECT));
    }
    return nullptr;
}

std::pair<uint32_t, uint32_t> brain::safe_preys(const game_bot&                  bot,
                                                const knowledge::hunting_ground& ground,
                                                const std::vector<std::string>&  keys) const
{
    auto count   = uint32_t{0};
    auto exp_sum = uint64_t{0};
    for (auto& prey : ground.preys)
    {
        if (this->dangerous(bot, prey, keys))
            continue;

        count   += prey.count;
        exp_sum += static_cast<uint64_t>(prey.exp) * prey.count;
    }

    if (count == 0)
        return {0, 0};

    return {count, static_cast<uint32_t>(exp_sum / count)};
}

std::optional<knowledge::hunting_ground> brain::select_ground(const game_bot& bot)
{
    auto now             = clock::now();
    auto keys            = this->experience_keys(bot);
    auto distances       = this->_knowledge.distances(bot.map(), bot.position(), bot.level());
    auto candidates      = std::vector<std::pair<double, const knowledge::hunting_ground*>>{};
    auto sparse          = std::vector<std::pair<double, const knowledge::hunting_ground*>>{};
    auto best            = 0.0;
    auto filtered        = std::array<uint32_t, 3>{};
    auto dangerous_count = uint32_t{0};
    auto multiplier      = this->_experience.multiplier();
    auto drop_multiplier = this->_experience.drop_multiplier();
    auto missing         = std::unordered_set<std::string>{};
    auto needed          = uint64_t{0};
    for (auto& [item, count] : this->reserved(bot))
    {
        auto owned = static_cast<uint32_t>(bot.get_item_count_by_name(item));
        if (owned >= count)
            continue;

        missing.insert(item);
        auto sellers = this->_knowledge.sellers(item);
        auto shop    = sellers.empty() ? nullptr : this->_knowledge.find_shop(sellers.front().npc);
        if (shop != nullptr && shop->sells.contains(item))
            needed += shop->sells.at(item) * (count - owned);
    }

    auto cls_now = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    for (auto lesson : this->_spellbook.pending(cls_now, bot, bot.level()))
    {
        needed += lesson->money;
    }
    auto money_weight = this->_persona.money_weight * (bot.money() < needed ? MONEY_NEED_WEIGHT : 1.0);

    auto dps =
        this->_dps >= 0.0 ? std::max(this->_dps, MIN_DPS) : static_cast<double>(std::max<uint8_t>(bot.level(), 1));
    auto cls = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    auto cap = std::numeric_limits<double>::infinity();
    if (table::ability[cls].contains(bot.level() + 1))
        cap = table::ability[cls][bot.level()].exp / 100.0 * 3.3 * multiplier + 1;
    for (auto& ground : this->_knowledge.hunting_grounds())
    {
        auto avoided = this->_avoided_grounds.find(ground.map);
        if (avoided != this->_avoided_grounds.end() && now < avoided->second)
        {
            filtered[0]++;
            continue;
        }

        auto hops = distances.find(ground.map);
        if (hops == distances.end())
        {
            filtered[1]++;
            continue;
        }

        auto count   = uint32_t{0};
        auto per_min = 0.0;
        auto useful  = 0.0;
        for (auto& prey : ground.preys)
        {
            if (this->dangerous(bot, prey, keys))
            {
                dangerous_count++;
                continue;
            }

            auto income = 0.0;
            for (auto& [item, percent] : prey.drops)
            {
                auto chance  = std::min(1.0, percent * drop_multiplier / 100.0);
                income      += chance * this->_knowledge.price(item);
                if (missing.contains(item))
                    useful += prey.count * chance;
            }

            auto record   = this->_experience.find(keys, prey.name);
            auto seconds  = record.has_value() ? record->seconds + record->loss * REST_SECONDS_FULL
                                               : APPROACH_SECONDS + static_cast<double>(prey.hp) / dps;
            auto gained   = std::min(prey.exp * multiplier, cap);
            count        += prey.count;
            per_min      += prey.count * (gained + income * money_weight) * 60.0 / std::max(seconds, 1.0);
        }

        if (count == 0)
        {
            filtered[2]++;
            continue;
        }

        auto travel   = static_cast<double>(hops->second) * this->_persona.step_delay.count() / 1000.0;
        auto expected = per_min / count * std::min<uint32_t>(count, 30);
        auto yield    = 1.0;
        auto observed = this->_yields.find(ground.map);
        if (observed != this->_yields.end() && expected > 0.0)
        {
            auto rate = (observed->second.first + expected * YIELD_PRIOR_MINUTES) /
                        (observed->second.second + YIELD_PRIOR_MINUTES);
            yield = std::clamp(rate / expected, YIELD_MIN, YIELD_MAX);
        }
        auto value = expected * yield * GROUND_STAY / (GROUND_STAY + travel) * (1.0 + MATERIAL_BONUS * useful / count);
        if (count < MIN_PREYS)
        {
            filtered[2]++;
            sparse.push_back({value, &ground});
        }
        else
        {
            best = std::max(best, value);
            candidates.push_back({value, &ground});
        }
    }

    if (candidates.empty())
    {
        candidates = std::move(sparse);
        for (auto& [value, _] : candidates)
        {
            best = std::max(best, value);
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    });

    auto returnable = std::vector<std::pair<double, const knowledge::hunting_ground*>>{};
    for (auto& candidate : candidates)
    {
        if (returnable.size() >= GROUND_CANDIDATES)
            break;

        auto ground = candidate.second;
        auto back   = ground->map == bot.map() ||
                    this->_knowledge.distances(ground->map, ground->center, bot.level()).contains(bot.map());
        if (back)
            returnable.push_back(candidate);
        else
            filtered[1]++;
    }
    candidates = std::move(returnable);

    auto detail           = Json::Value{};
    detail["event"]       = "grounds";
    detail["level"]       = bot.level();
    detail["dps"]         = dps;
    detail["cap"]         = std::isinf(cap) ? -1.0 : cap;
    detail["hp"]          = std::max(bot.base_hp(), bot.hp());
    detail["avoided"]     = filtered[0];
    detail["unreachable"] = filtered[1];
    detail["unsafe"]      = filtered[2];
    detail["dangerous"]   = dangerous_count;
    for (auto& [value, ground] : candidates)
    {
        auto candidate     = Json::Value{};
        candidate["map"]   = ground->map;
        candidate["value"] = value;
        candidate["steps"] = distances.at(ground->map);
        detail["candidates"].append(candidate);
    }
    this->_trace.write(bot.name(), detail);

    for (auto& [weight, _] : candidates)
    {
        weight = std::exp((weight / best - 1.0) / std::max(this->_persona.temperature, 0.01));
    }

    if (candidates.empty())
    {
        fb::logger::debug("load brain: bot={} no ground level={} hp={} avoided={} unreachable={} unsafe={}",
                          bot.name(),
                          bot.level(),
                          std::max(bot.base_hp(), bot.hp()),
                          filtered[0],
                          filtered[1],
                          filtered[2]);
        return std::nullopt;
    }

    auto weights = std::vector<double>{};
    for (auto& [weight, _] : candidates)
    {
        weights.push_back(weight);
    }

    auto index  = std::discrete_distribution<size_t>(weights.begin(), weights.end())(this->_random);
    auto ground = candidates.at(index).second;
    fb::logger::debug("load brain: bot={} ground={} steps={} candidates={}",
                      bot.name(),
                      ground->map,
                      distances.at(ground->map),
                      candidates.size());
    return *ground;
}

std::optional<knowledge::hunting_ground> brain::find_ground(const game_bot& bot, const std::string& suffix) const
{
    auto maps      = table::map;
    auto keys      = this->experience_keys(bot);
    auto distances = this->_knowledge.distances(bot.map(), bot.position(), bot.level());
    auto nearest   = std::optional<knowledge::hunting_ground>{};
    auto hops      = std::numeric_limits<uint32_t>::max();
    for (auto& ground : this->_knowledge.hunting_grounds())
    {
        auto model = maps->find(ground.map);
        if (model == nullptr || model->name.ends_with(suffix) == false ||
            this->safe_preys(bot, ground, keys).first == 0)
            continue;

        auto found = distances.find(ground.map);
        if (found != distances.end() && found->second < hops)
        {
            hops    = found->second;
            nearest = ground;
        }
    }
    return nearest;
}

const tutorial::step* brain::find_step(const game_bot& bot) const
{
    auto now       = clock::now();
    auto available = [this, now](const tutorial::step& step) {
        auto avoided = this->_avoided_steps.find(step.item);
        return avoided == this->_avoided_steps.end() || now >= avoided->second;
    };

    for (auto& step : this->_tutorial.steps)
    {
        if (bot.has_item_by_name(step.item) && available(step))
            return &step;
    }

    if (this->_tutorial.steps.empty() == false && bot.map() == this->_tutorial.giver_map &&
        available(this->_tutorial.steps.front()))
        return &this->_tutorial.steps.front();

    return nullptr;
}

std::optional<knowledge::npc_spot> brain::nearest(const game_bot&                         bot,
                                                  const std::vector<knowledge::npc_spot>& spots) const
{
    auto distances = this->_knowledge.distances(bot.map(), bot.position(), bot.level());
    auto found     = std::optional<knowledge::npc_spot>{};
    auto hops      = std::numeric_limits<uint32_t>::max();
    for (auto& spot : spots)
    {
        auto distance = distances.find(spot.map);
        if (distance != distances.end() && distance->second < hops)
        {
            hops  = distance->second;
            found = spot;
        }
    }
    return found;
}

async::task<brain::result> brain::revive(game_bot& bot)
{
    auto nearest = this->nearest(bot, this->_knowledge.revive_spots());
    if (nearest.has_value() == false)
        co_return co_await this->escape(bot);

    auto spot    = nearest.value();
    auto arrived = co_await this->travel(bot, spot.map, spot.position, TALK_RANGE);
    if (arrived == result::FAILED)
        co_return co_await this->escape(bot);

    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto oid = uint32_t{0};
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (entry.position == spot.position)
            oid = entry.oid;
    }

    if (oid == 0)
    {
        fb::logger::debug("load brain: bot={} revive npc not seen map={} spot=({},{})",
                          bot.name(),
                          spot.map,
                          spot.position.x,
                          spot.position.y);
        co_return result::FAILED;
    }

    auto talked = co_await this->talk(bot, oid);
    if (talked != result::SUCCEEDED)
    {
        fb::logger::debug("load brain: bot={} revive talk failed map={}", bot.name(), spot.map);
        co_return talked;
    }

    auto deadline = clock::now() + DIALOG_TIMEOUT;
    while (bot.state() == STATE::GHOST && clock::now() < deadline)
    {
        co_await bot.thread()->sleep(100ms);
    }

    if (bot.state() == STATE::GHOST)
        co_return result::FAILED;

    if (bot.hp() <= APPRECIATE_HP)
        std::ignore = co_await this->appreciate(bot);
    co_return result::SUCCEEDED;
}

bool brain::leader_hunting() const
{
    auto& grounds = this->_knowledge.hunting_grounds();
    return std::any_of(grounds.begin(), grounds.end(), [this](const auto& ground) {
        return ground.map == this->_leader_map;
    });
}

bool brain::hostile(const game_bot& bot) const
{
    auto keys = this->experience_keys(bot);
    for (auto& ground : this->_knowledge.hunting_grounds())
    {
        if (ground.map != bot.map())
            continue;

        return std::any_of(ground.preys.begin(), ground.preys.end(), [&](const auto& prey) {
            return this->dangerous(bot, prey, keys);
        });
    }
    return false;
}

async::task<brain::result> brain::rest(game_bot& bot)
{
    if (bot.state() != STATE::GHOST && clock::now() >= this->_escaped_until && this->hostile(bot))
    {
        this->_escaped_until = clock::now() + ESCAPE_COOLDOWN;
        auto escaped         = co_await this->escape(bot);
        if (escaped == result::INTERRUPTED)
            co_return escaped;
    }

    auto deadline = clock::now() + REST_TIMEOUT;
    auto last_hp  = bot.hp();
    while (this->alive(bot) && this->_interrupted == false)
    {
        auto max_hp = std::max(bot.base_hp(), bot.hp());
        if (max_hp == 0 || bot.hp() >= max_hp * this->_persona.rest_until)
            co_return result::SUCCEEDED;

        if (clock::now() >= deadline)
            co_return result::FAILED;

        auto shrine = std::optional<uint32_t>{};
        if (bot.hp() <= APPRECIATE_HP)
        {
            auto spot = this->nearest(bot, this->_knowledge.revive_spots());
            if (spot.has_value())
            {
                auto distances = this->_knowledge.distances(bot.map(), bot.position(), bot.level());
                shrine         = distances.at(spot->map);
            }
        }

        auto has_food =
            std::any_of(this->_supplies.heals.begin(), this->_supplies.heals.end(), [&bot](const auto& heal) {
                return bot.has_item_by_name(heal.first);
            });

        if (shrine.has_value() && (has_food == false || shrine.value() <= APPRECIATE_STEPS))
        {
            auto appreciated = co_await this->appreciate(bot);
            if (appreciated != result::SUCCEEDED)
                co_return appreciated;
        }
        else if (bot.hp() < last_hp)
        {
            co_await this->retreat(bot);
        }
        else if (co_await this->mend(bot) == false && co_await this->eat(bot) == false)
        {
            co_await bot.thread()->sleep(this->paced(REST_IDLE));
        }

        last_hp = bot.hp();
    }
    co_return result::INTERRUPTED;
}

async::task<bool> brain::mend(game_bot& bot)
{
    auto now  = clock::now();
    auto best = std::optional<std::pair<std::string, spellbook::effect>>{};
    for (auto& [_, spell] : bot.spells())
    {
        auto effect = this->_spellbook.find(spell.name);
        if (effect == nullptr || effect->role != spellbook::ROLE::HEAL || effect->weight <= 0.0 ||
            bot.mp() < effect->mp)
            continue;

        auto wait = this->_spell_waits.find(spell.name);
        if (wait != this->_spell_waits.end() && now < wait->second)
            continue;

        if (best.has_value() == false || effect->hp > best->second.hp)
            best = std::pair{spell.name, *effect};
    }

    if (best.has_value() == false)
        co_return false;

    auto target = best->second.type == SPELL_TYPE::TARGET ? bot.oid() : 0u;
    auto cast   = co_await this->cast(bot, best->first, {}, target);
    if (cast == false)
        this->_spell_waits.insert_or_assign(best->first, clock::now() + SPELL_RETRY);
    co_return cast;
}

async::task<brain::result> brain::appreciate(game_bot& bot)
{
    auto spot = this->nearest(bot, this->_knowledge.revive_spots());
    if (spot.has_value() == false)
        co_return result::FAILED;

    auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto since = clock::now();
    auto hp    = bot.hp();
    bot.send(*bot.make<game_reqs::chat>(false, "감사합니다"));
    auto reason = co_await this->expect(
        bot,
        since,
        {},
        [&bot, hp]() {
            return bot.hp() > hp;
        },
        ACTION_TIMEOUT);

    auto detail      = Json::Value{};
    detail["before"] = hp;
    detail["npc"]    = spot->npc;
    this->report(bot, "appreciate", reason, detail);

    if (reason == "INTERRUPTED")
        co_return result::INTERRUPTED;
    co_return reason.empty() ? result::SUCCEEDED : result::FAILED;
}

async::task<void> brain::retreat(game_bot& bot)
{
    auto position = bot.position();
    auto threat   = std::optional<point<uint16_t>>{};
    auto nearest  = std::numeric_limits<int>::max();
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (entry.look <= knowledge::MOB_LOOK_OFFSET || entry.look > ITEM_LOOK_MIN ||
            this->_knowledge.npc_at(bot.map(), entry.position))
            continue;

        auto distance = std::abs(entry.position.x - position.x) + std::abs(entry.position.y - position.y);
        if (distance < nearest)
        {
            nearest = distance;
            threat  = entry.position;
        }
    }

    auto grid = this->_knowledge.find_grid(bot.map());
    if (threat.has_value() == false || grid == nullptr)
    {
        co_await bot.thread()->sleep(this->paced(1000ms));
        co_return;
    }

    auto away = [](int from, int threat, int size) {
        auto step = from >= threat ? RETREAT_RANGE : -RETREAT_RANGE;
        return static_cast<uint16_t>(std::clamp(from + step, 0, size - 1));
    };
    auto target = point<uint16_t>(away(position.x, threat->x, grid->width), away(position.y, threat->y, grid->height));
    if (co_await this->walk(bot, target, RETREAT_RANGE / 2) == result::FAILED)
        co_await bot.thread()->sleep(this->paced(1000ms));
}

async::task<brain::result> brain::learn(game_bot& bot)
{
    auto step = this->find_step(bot);
    if (step == nullptr)
        co_return result::FAILED;

    this->_tutorial_grace = clock::now() + TUTORIAL_GRACE;
    auto item             = step->item;
    auto before           = std::pair{step, bot.has_item_by_name(item)};
    auto outcome          = result::SUCCEEDED;
    auto impossible       = false;
    if (before.second == false)
    {
        auto spot = this->_knowledge.find_npc(this->_tutorial.giver_map, this->_tutorial.giver_npc);
        if (spot.has_value())
            outcome = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
        else
            impossible = true;

        auto oid = uint32_t{0};
        if (outcome == result::SUCCEEDED)
        {
            for (auto& [_, entry] : bot.sight.entries())
            {
                if (entry.position == spot->position)
                    oid = entry.oid;
            }
        }

        if (impossible == false && outcome == result::SUCCEEDED && oid == 0)
            impossible = true;
        else if (impossible == false && outcome == result::SUCCEEDED)
            outcome = co_await this->talk(bot, oid);
    }
    else
    {
        outcome = co_await this->use(bot, item);
        if (outcome == result::SUCCEEDED && step->task == tutorial::TASK::USE)
        {
            for (uint32_t i = 0; i < step->count && outcome == result::SUCCEEDED; i++)
            {
                if (bot.has_item_by_name(step->target))
                    outcome = co_await this->use(bot, step->target);
            }
        }
        else if (outcome == result::SUCCEEDED &&
                 (step->task == tutorial::TASK::TRAVEL || step->task == tutorial::TASK::HUNT))
        {
            auto ground = this->find_ground(bot, step->target);
            if (ground.has_value())
                outcome = co_await this->travel(bot, ground->map, ground->center, 4);
            else
                impossible = true;

            if (impossible == false && outcome == result::SUCCEEDED && step->task == tutorial::TASK::HUNT)
                outcome = co_await this->farm(bot, ground.value(), TUTORIAL_SLICE);
        }

        if (impossible == false && outcome == result::SUCCEEDED && step->task != tutorial::TASK::NONE)
        {
            this->_tutorial_grace = clock::now() + TUTORIAL_GRACE;
            outcome               = co_await this->use(bot, item);
        }
    }

    if (outcome == result::INTERRUPTED)
        co_return outcome;

    this->_tutorial_grace = clock::now() + TUTORIAL_GRACE;
    auto deadline         = clock::now() + DIALOG_TIMEOUT;
    auto current          = std::pair{this->find_step(bot), bot.has_item_by_name(item)};
    while (this->alive(bot) && current == before && clock::now() < deadline)
    {
        co_await bot.thread()->sleep(100ms);
        current = std::pair{this->find_step(bot), bot.has_item_by_name(item)};
    }

    if (current != before)
    {
        this->_step_attempts.erase(item);
        fb::logger::debug("load brain: bot={} tutorial step={} done", bot.name(), item);
        co_return result::SUCCEEDED;
    }

    if (impossible == false && outcome == result::FAILED)
        co_return outcome;

    auto attempts = ++this->_step_attempts[item];
    if (impossible || attempts >= STEP_ATTEMPTS)
    {
        this->_step_attempts.erase(item);
        this->_avoided_steps.insert_or_assign(item, clock::now() + STEP_AVOID);
        fb::logger::warn("load brain: bot={} tutorial step={} abandoned", bot.name(), item);
        co_return result::FAILED;
    }
    co_return outcome;
}

async::task<brain::result> brain::join(game_bot& bot)
{
    auto users = std::vector<game_bot::user_data>{};
    auto seq   = bot.users(users);
    bot.send(*bot.make<game_reqs::user_list>());
    auto deadline = clock::now() + DIALOG_TIMEOUT;
    while (this->alive(bot) && bot.users(users) == seq && clock::now() < deadline)
    {
        co_await bot.thread()->sleep(100ms);
    }

    if (bot.users(users) != seq)
    {
        auto population = std::array<uint32_t, CLASS_COUNT>{};
        for (auto& user : users)
        {
            if (user.cls < CLASS_COUNT && user.name != bot.name())
                population[user.cls]++;
        }
        this->_census.update(population);
    }

    auto selected = this->_census.reserve(bot.name(), this->_job.demand, this->_random);
    auto npc      = selected.has_value() ? this->_job.npcs.find(selected.value()) : this->_job.npcs.end();
    if (npc == this->_job.npcs.end())
        co_return result::FAILED;

    if (bot.get_item_count_by_name(this->_job.item) < this->_job.count)
    {
        auto ground = this->find_ground(bot, this->_job.ground);
        if (ground.has_value() == false)
            co_return result::FAILED;

        auto arrived = co_await this->travel(bot, ground->map, ground->center, 4);
        if (arrived != result::SUCCEEDED)
            co_return arrived;

        co_return co_await this->farm(bot, ground.value(), TUTORIAL_SLICE);
    }

    auto spot = this->nearest(bot, this->_knowledge.npc_spots(npc->second));
    if (spot.has_value() == false)
        co_return result::FAILED;

    auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto oid = uint32_t{0};
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (entry.position == spot->position)
            oid = entry.oid;
    }

    if (oid == 0)
        co_return result::FAILED;

    auto talked = co_await this->talk(bot, oid);
    if (talked != result::SUCCEEDED)
        co_return talked;

    deadline = clock::now() + DIALOG_TIMEOUT;
    bot.send(*bot.make<game_reqs::self_info>());
    while (this->alive(bot) && this->_knowledge.find_class(bot.class_name()) == CLASS::NONE && clock::now() < deadline)
    {
        co_await bot.thread()->sleep(100ms);
    }

    auto joined = this->_knowledge.find_class(bot.class_name());
    if (joined.has_value() == false || joined.value() == CLASS::NONE)
        co_return result::FAILED;

    this->_census.release(bot.name());
    fb::logger::info("load brain: bot={} joined class={}", bot.name(), static_cast<int>(joined.value()));
    co_return result::SUCCEEDED;
}

std::string brain::stack_name(const std::string& name)
{
    auto space = name.rfind(' ');
    if (space != std::string::npos && name.ends_with("개"))
        return name.substr(0, space);

    return name;
}

std::unordered_map<std::string, uint32_t> brain::owned(const game_bot& bot) const
{
    auto counts = std::unordered_map<std::string, uint32_t>{};
    for (auto& [_, item] : bot.items())
    {
        counts[brain::stack_name(item.name)] += std::max<uint32_t>(item.count, 1);
    }
    return counts;
}

std::vector<std::pair<std::string, uint32_t>> brain::sellable(const game_bot& bot) const
{
    auto items    = std::vector<std::pair<std::string, uint32_t>>{};
    auto reserved = this->reserved(bot);
    for (auto& [name, count] : this->owned(bot))
    {
        auto kept = std::any_of(this->_supplies.keeps.begin(), this->_supplies.keeps.end(), [&name](const auto& keep) {
            return name.find(keep) != std::string::npos;
        });
        if (kept || this->_knowledge.buyable(name) == false || this->_market.wanted(name, bot.name()))
            continue;

        auto keep = this->held(bot, name, reserved);
        if (keep == 0)
            items.push_back({name, 0});
        else if (count > keep)
            items.push_back({name, count - keep});
    }
    return items;
}

std::unordered_map<std::string, uint32_t> brain::reserved(const game_bot& bot) const
{
    auto result = std::unordered_map<std::string, uint32_t>{};
    auto cls    = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    auto level  = static_cast<uint8_t>(std::min(255, bot.level() + this->_spellbook.lookahead));
    for (auto lesson : this->_spellbook.pending(cls, bot, level))
    {
        for (auto& [item, count] : lesson->materials)
        {
            result[item] += count;
        }
    }

    auto quest = this->choose_quest(bot);
    if (quest != nullptr)
    {
        for (auto& [item, count] : quest->consumes)
        {
            result[item] += count;
        }
    }
    return result;
}

bool brain::overstocked(const game_bot& bot) const
{
    auto models   = table::item;
    auto reserved = this->reserved(bot);
    for (auto& [_, item] : bot.items())
    {
        auto model = models->name2item(brain::stack_name(item.name));
        if (model == nullptr || item.count < std::max<uint32_t>(model->capacity, 1))
            continue;

        if (item.count > this->held(bot, brain::stack_name(item.name), reserved))
            return true;
    }
    return false;
}

bool brain::underfunded(const game_bot& bot) const
{
    auto cls = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    if (cls == CLASS::NONE)
        return false;

    for (auto lesson : this->_spellbook.pending(cls, bot, bot.level()))
    {
        auto cost = this->tuition(bot, *lesson);
        if (cost.has_value() == false)
            continue;

        if (bot.money() >= cost.value())
            return false;

        auto owned = this->owned(bot);
        auto worth = uint64_t{0};
        for (auto& [name, count] : this->sellable(bot))
        {
            auto best = uint64_t{0};
            for (auto& [_, shop] : this->_knowledge.shops())
            {
                auto found = shop.buys.find(name);
                if (found != shop.buys.end())
                    best = std::max(best, found->second);
            }
            auto have  = owned.find(name);
            auto sold  = count > 0 ? count : (have != owned.end() ? have->second : 0u);
            worth     += best * sold;
        }
        return worth >= std::min<uint64_t>(cost.value() - bot.money(), FUNDING_MIN);
    }
    return false;
}

uint32_t brain::held(const game_bot&                                  bot,
                     const std::string&                               item,
                     const std::unordered_map<std::string, uint32_t>& reserved) const
{
    auto stock   = this->_supplies.stocks.find(item);
    auto reserve = reserved.find(item);
    auto keep    = std::max(stock != this->_supplies.stocks.end() ? stock->second : 0u,
                         reserve != reserved.end() ? reserve->second : 0u);
    for (auto& restock : this->_supplies.restocks)
    {
        if (restock.item != item)
            continue;

        if ((restock.unless.empty() || bot.get_spell_slot_by_name(restock.unless) == 0xFF) &&
            (restock.caster == false || this->caster(bot)))
            keep = std::max(keep, restock.count);
    }
    return keep;
}

async::task<brain::result> brain::sell(game_bot& bot)
{
    auto items = this->sellable(bot);
    auto names = std::vector<std::string>{};
    for (auto& [name, _] : items)
    {
        names.push_back(name);
    }

    auto buyers = this->_knowledge.buyers(names);
    std::erase_if(buyers, [this, &names](const auto& buyer) {
        auto shop     = this->_knowledge.find_shop(buyer.npc);
        auto rejected = this->_rejected_trades.find(buyer.npc);
        return shop == nullptr || std::none_of(names.begin(), names.end(), [&](const auto& name) {
                   return shop->buys.contains(name) &&
                          (rejected == this->_rejected_trades.end() || rejected->second.contains(name) == false);
               });
    });

    auto spot = this->nearest(bot, buyers);
    if (spot.has_value() == false)
    {
        this->_cooldowns[static_cast<size_t>(GOAL::SELL)] = clock::now() + SHOP_AVOID;
        co_return result::FAILED;
    }

    auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto shop = this->_knowledge.find_shop(spot->npc);
    if (shop == nullptr)
        co_return result::FAILED;

    auto stock = [this, &bot]() {
        auto owned = this->owned(bot);
        return std::accumulate(owned.begin(), owned.end(), uint64_t{0}, [](auto sum, const auto& pair) {
            return sum + pair.second;
        });
    };
    auto before = stock();
    for (auto& [item, count] : items)
    {
        auto& rejected = this->_rejected_trades[spot->npc];
        if (shop->buys.contains(item) == false || rejected.contains(item))
            continue;

        auto text  = count == 0 ? item + " 다 판다" : std::format("{} {}개 판다", item, count);
        auto since = clock::now();
        auto total = stock();
        auto money = bot.money();
        bot.send(*bot.make<game_reqs::chat>(false, text));
        auto reason = co_await this->expect(
            bot,
            since,
            {"SHOP_NOT_BUY", "NOT_OWNED", "COUNT_SHORT", "HOLD_FULL"},
            [&]() {
                return stock() != total || bot.money() != money;
            },
            ACTION_TIMEOUT);

        auto detail    = Json::Value{};
        detail["item"] = fb::UTF8(item, fb::PLATFORM::WINDOWS);
        detail["npc"]  = spot->npc;
        this->report(bot, "sell", reason, detail);

        if (reason == "INTERRUPTED")
            co_return result::INTERRUPTED;
        else if (reason == "SHOP_NOT_BUY")
            rejected.insert(item);
    }

    auto sold = stock() < before;
    if (sold)
        this->_sell_urgent = false;
    else
        this->_cooldowns[static_cast<size_t>(GOAL::SELL)] = clock::now() + SHOP_AVOID;
    co_return sold ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::supply(game_bot& bot)
{
    auto restock = this->shortage(bot);
    if (restock == nullptr)
        co_return result::FAILED;

    auto keep   = bot.get_item_count_by_name(restock->item) > 0 ? this->savings(bot) : 0u;
    auto bought = co_await this->buy(bot, restock->item, restock->count, keep);
    if (bought == result::FAILED)
        this->_cooldowns[static_cast<size_t>(GOAL::SUPPLY)] = clock::now() + SHOP_AVOID;
    co_return bought;
}

uint64_t brain::savings(const game_bot& bot) const
{
    auto cls = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    if (cls == CLASS::NONE)
        return 0;

    for (auto lesson : this->_spellbook.pending(cls, bot, bot.level()))
    {
        auto cost = this->tuition(bot, *lesson);
        if (cost.has_value())
            return cost.value();
    }
    return 0;
}

std::optional<uint64_t> brain::tuition(const game_bot& bot, const spellbook::lesson& lesson) const
{
    auto cost = static_cast<uint64_t>(lesson.money);
    for (auto& [item, count] : lesson.materials)
    {
        auto owned = static_cast<uint32_t>(bot.get_item_count_by_name(item));
        if (owned >= count)
            continue;

        if (this->_knowledge.sellers(item).empty())
            return std::nullopt;

        cost += this->_knowledge.price(item) * (count - owned);
    }
    return cost;
}

const spellbook::lesson* brain::affordable(const game_bot& bot) const
{
    auto cls = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    if (cls == CLASS::NONE)
        return nullptr;

    for (auto lesson : this->_spellbook.pending(cls, bot, bot.level()))
    {
        auto cost = this->tuition(bot, *lesson);
        if (cost.has_value() && bot.money() >= cost.value())
            return lesson;
    }
    return nullptr;
}

const supplies::restock* brain::shortage(const game_bot& bot) const
{
    for (auto& restock : this->_supplies.restocks)
    {
        if (restock.unless.empty() == false && bot.get_spell_slot_by_name(restock.unless) != 0xFF)
            continue;

        if (restock.caster && this->caster(bot) == false)
            continue;

        auto owned = bot.get_item_count_by_name(restock.item);
        if (owned >= (restock.count + 1) / 2)
            continue;

        auto price = this->_knowledge.price(restock.item);
        if (owned > 0 && price > 0 && bot.money() < this->savings(bot) + price)
            continue;

        return &restock;
    }
    return nullptr;
}

bool brain::caster(const game_bot& bot) const
{
    for (auto& [_, spell] : bot.spells())
    {
        auto effect = this->_spellbook.find(spell.name);
        if (effect != nullptr && effect->mp > 0 &&
            (effect->role == spellbook::ROLE::HEAL || effect->role == spellbook::ROLE::ATTACK ||
             effect->role == spellbook::ROLE::AREA || effect->role == spellbook::ROLE::DEBUFF))
            return true;
    }
    return false;
}

async::task<brain::result> brain::study(game_bot& bot)
{
    auto cls    = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    auto lesson = this->affordable(bot);
    if (lesson == nullptr)
        co_return result::FAILED;

    auto detail = Json::Value{};
    for (auto& name : lesson->names)
    {
        detail["names"].append(fb::UTF8(name, fb::PLATFORM::WINDOWS));
    }
    detail["level"] = lesson->level;

    for (auto& [item, count] : lesson->materials)
    {
        if (bot.get_item_count_by_name(item) >= count)
            continue;

        auto bought = co_await this->buy(bot, item, count);
        if (bought == result::INTERRUPTED)
            co_return bought;

        if (bot.get_item_count_by_name(item) < count)
        {
            detail["material"] = fb::UTF8(item, fb::PLATFORM::WINDOWS);
            this->report(bot, "learn", "MATERIAL_SHORT", detail);
            this->_cooldowns[static_cast<size_t>(GOAL::LEARN)] = clock::now() + SHOP_AVOID;
            co_return result::FAILED;
        }
    }

    if (bot.money() < lesson->money)
    {
        this->report(bot, "learn", "MONEY_SHORT", detail);
        this->_cooldowns[static_cast<size_t>(GOAL::LEARN)] = clock::now() + SHOP_AVOID;
        co_return result::FAILED;
    }

    auto guild = this->_spellbook.guilds.find(cls);
    auto spot  = this->nearest(bot, this->_knowledge.npc_spots(guild->second.npc));
    if (spot.has_value() == false)
        co_return result::FAILED;

    auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto oid = uint32_t{0};
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (entry.position == spot->position)
            oid = entry.oid;
    }

    if (oid == 0)
        co_return result::FAILED;

    auto picks = std::vector<std::string>{"마법 배우기"};
    picks.insert(picks.end(), lesson->names.begin(), lesson->names.end());
    picks.push_back("예");

    auto talked = co_await this->talk(bot, oid, picks);
    if (talked == result::INTERRUPTED)
        co_return talked;

    auto learned = std::any_of(lesson->names.begin(), lesson->names.end(), [&bot](const auto& name) {
        return bot.get_spell_slot_by_name(name) != 0xFF;
    });
    this->report(bot, "learn", learned ? std::string{} : std::string{"NOT_LEARNED"}, detail);
    if (learned == false)
        this->_cooldowns[static_cast<size_t>(GOAL::LEARN)] = clock::now() + SHOP_AVOID;
    co_return learned ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::buy(game_bot& bot, const std::string& item, uint32_t total, uint64_t keep)
{
    auto spot = this->nearest(bot, this->_knowledge.sellers(item));
    if (spot.has_value() == false)
        co_return result::FAILED;

    auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto shop      = this->_knowledge.find_shop(spot->npc);
    auto price     = shop != nullptr && shop->sells.contains(item) ? shop->sells.at(item) : 0;
    auto owned     = static_cast<uint32_t>(bot.get_item_count_by_name(item));
    auto need      = total > owned ? total - owned : 0u;
    auto spendable = bot.money() > keep ? bot.money() - keep : 0u;
    if (price > 0)
        need = std::min<uint32_t>(need, static_cast<uint32_t>(spendable / price));

    if (need == 0)
        co_return result::FAILED;

    auto since = clock::now();
    bot.send(*bot.make<game_reqs::chat>(false, std::format("{} {}개 줘", item, need)));
    auto reason = co_await this->expect(
        bot,
        since,
        {"MONEY_SHORT", "SHOP_NOT_SELL", "INVENTORY_FULL", "HOLD_FULL"},
        [&bot, &item, owned]() {
            return bot.get_item_count_by_name(item) != owned;
        },
        ACTION_TIMEOUT);

    auto detail     = Json::Value{};
    detail["item"]  = fb::UTF8(item, fb::PLATFORM::WINDOWS);
    detail["count"] = need;
    detail["npc"]   = spot->npc;
    this->report(bot, "buy", reason, detail);

    if (reason == "INVENTORY_FULL" || reason == "HOLD_FULL")
        this->_sell_urgent = true;

    co_return reason.empty() ? result::SUCCEEDED : result::FAILED;
}

async::task<bool> brain::eat(game_bot& bot)
{
    auto max_hp   = std::max(bot.base_hp(), bot.hp());
    auto deficit  = max_hp > bot.hp() ? max_hp - bot.hp() : 0u;
    auto chosen   = std::optional<std::pair<std::string, uint32_t>>{};
    auto reserved = this->reserved(bot);
    for (auto& [name, hp] : this->_supplies.heals)
    {
        auto reserve = reserved.find(name);
        if (bot.get_item_count_by_name(name) <= (reserve != reserved.end() ? reserve->second : 0u))
            continue;

        auto better = chosen.has_value() == false || (hp <= deficit && hp > chosen->second) ||
                      (chosen->second > deficit && hp < chosen->second);
        if (better)
            chosen = std::pair{name, hp};
    }

    if (chosen.has_value() == false || deficit == 0)
        co_return false;

    auto since = clock::now();
    auto hp    = bot.hp();
    bot.send(*bot.make<game_reqs::item_active>(bot.get_item_slot_by_name(chosen->first)));
    auto reason = co_await this->expect(
        bot,
        since,
        {"GHOST_FORBIDDEN", "RIDING_FORBIDDEN", "DISGUISE_FORBIDDEN"},
        [&bot, hp]() {
            return bot.hp() != hp;
        },
        this->_persona.attack_delay);

    if (reason.empty() == false && reason != "TIMEOUT")
    {
        auto detail    = Json::Value{};
        detail["item"] = fb::UTF8(chosen->first, fb::PLATFORM::WINDOWS);
        this->report(bot, "eat", reason, detail);
        co_return false;
    }
    co_return true;
}

async::task<bool> brain::drink(game_bot& bot)
{
    auto max_mp  = std::max(bot.base_mp(), bot.mp());
    auto deficit = max_mp > bot.mp() ? max_mp - bot.mp() : 0u;
    auto chosen  = std::optional<std::pair<std::string, uint32_t>>{};
    for (auto& [name, mp] : this->_supplies.drinks)
    {
        if (bot.has_item_by_name(name) == false)
            continue;

        if (chosen.has_value() == false || (mp <= deficit && mp > chosen->second))
            chosen = std::pair{name, mp};
    }

    if (chosen.has_value() == false || deficit < chosen->second)
        co_return false;

    auto since = clock::now();
    auto mp    = bot.mp();
    bot.send(*bot.make<game_reqs::item_active>(bot.get_item_slot_by_name(chosen->first)));
    auto reason = co_await this->expect(
        bot,
        since,
        {"GHOST_FORBIDDEN", "RIDING_FORBIDDEN", "DISGUISE_FORBIDDEN"},
        [&bot, mp]() {
            return bot.mp() != mp;
        },
        this->_persona.attack_delay);

    auto detail    = Json::Value{};
    detail["item"] = fb::UTF8(chosen->first, fb::PLATFORM::WINDOWS);
    detail["mp"]   = mp;
    this->report(bot, "drink", reason, detail);
    co_return reason.empty();
}

async::task<bool> brain::enchant(game_bot& bot)
{
    if (bot.weapon_dress() == 0 || bot.weapon_dress() == this->_enchanted)
        co_return false;

    auto art = std::optional<std::pair<std::string, uint32_t>>{};
    for (auto& [_, spell] : bot.spells())
    {
        auto effect = this->_spellbook.find(spell.name);
        if (effect != nullptr && effect->role == spellbook::ROLE::ENCHANT && effect->weight > 0.0 &&
            bot.mp() >= effect->mp && (art.has_value() == false || effect->id > art->second))
            art = std::pair{spell.name, effect->id};
    }

    if (art.has_value() == false)
        co_return false;

    auto names = std::vector<std::string>{};
    auto items = table::item;
    for (auto& [_, item] : items)
    {
        auto weapon = dynamic_cast<const fb::model::weapon*>(&item);
        if (weapon != nullptr && weapon->dress == bot.weapon_dress())
            names.push_back(weapon->name);
    }

    this->_enchanted = bot.weapon_dress();
    if (names.size() != 1)
        co_return false;

    co_return co_await this->cast(bot, art->first, names.front(), 0);
}

async::task<brain::result> brain::reclaim(game_bot& bot)
{
    auto map      = this->_remains_map;
    auto position = this->_remains_position;
    auto arrived  = co_await this->travel(bot, map, position, 0);
    if (arrived == result::INTERRUPTED)
        co_return arrived;

    this->_remains_map = 0xFFFF;
    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto money  = bot.money();
    auto picked = 0u;
    auto reason = std::string{};
    for (auto i = 0; i < RECLAIM_LOOTS; i++)
    {
        auto remains = std::vector<uint32_t>{};
        for (auto& [oid, entry] : bot.sight.entries())
        {
            if (entry.look > ITEM_LOOK_MIN && entry.position == position)
                remains.push_back(oid);
        }

        if (remains.empty())
            break;

        auto since  = clock::now();
        auto before = bot.items();
        auto cash   = bot.money();
        bot.send(*bot.make<game_reqs::loot>(false));
        reason = co_await this->expect(
            bot,
            since,
            {"HOLD_FULL", "INVENTORY_FULL"},
            [&]() {
                return bot.money() != cash || bot.items().size() != before.size() ||
                       std::any_of(remains.begin(), remains.end(), [&bot](auto oid) {
                           return bot.sight.entries().contains(oid) == false;
                       });
            },
            ACTION_TIMEOUT);

        if (reason.empty() == false)
            break;
        picked++;
    }

    auto detail      = Json::Value{};
    detail["map"]    = map;
    detail["picked"] = picked;
    detail["money"]  = bot.money() > money ? bot.money() - money : 0u;
    this->report(bot, "reclaim", reason, detail);

    if (reason == "INTERRUPTED")
        co_return result::INTERRUPTED;
    else if (reason == "INVENTORY_FULL")
        this->_sell_urgent = true;
    co_return picked > 0 ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::hunt(game_bot& bot)
{
    auto now = clock::now();
    if (this->_ground.has_value())
    {
        auto avoided = this->_avoided_grounds.find(this->_ground->map);
        if (avoided != this->_avoided_grounds.end() && now < avoided->second)
            this->_ground.reset();
    }

    if (this->_ground.has_value() == false)
        this->_ground = this->select_ground(bot);

    if (this->_ground.has_value() == false && now >= this->_escaped_until)
    {
        this->_escaped_until = now + ESCAPE_COOLDOWN;
        co_return co_await this->escape(bot);
    }
    else if (this->_ground.has_value() == false)
    {
        co_return result::FAILED;
    }

    auto ground  = this->_ground.value();
    auto arrived = co_await this->travel(bot, ground.map, ground.center, 4);
    if (arrived == result::FAILED && bot.map() == ground.map)
    {
        arrived = result::SUCCEEDED;
    }
    else if (arrived == result::FAILED)
    {
        this->_avoided_grounds.insert_or_assign(ground.map, clock::now() + GROUND_AVOID);
        this->_ground.reset();
    }

    if (arrived != result::SUCCEEDED)
        co_return arrived;

    auto  started  = clock::now();
    auto  exp      = bot.exp();
    auto  farmed   = co_await this->farm(bot, ground, HUNT_SLICE);
    auto  gained   = bot.exp() > exp ? bot.exp() - exp : 0u;
    auto  minutes  = std::chrono::duration<double>(clock::now() - started).count() / 60.0;
    auto& yield    = this->_yields[ground.map];
    yield.first   += static_cast<double>(gained);
    yield.second  += minutes;
    if (farmed != result::SUCCEEDED)
        co_return farmed;

    fb::logger::debug("load brain: bot={} farmed ground={} pos=({},{}) exp={}",
                      bot.name(),
                      ground.map,
                      bot.position().x,
                      bot.position().y,
                      gained);
    if (clock::now() - started >= HUNT_GRACE &&
        gained < this->safe_preys(bot, ground, this->experience_keys(bot)).second * 3)
    {
        this->_avoided_grounds.insert_or_assign(ground.map, clock::now() + GROUND_AVOID);
        this->_ground.reset();
        co_return result::FAILED;
    }
    co_return result::SUCCEEDED;
}

async::task<brain::result> brain::farm(game_bot&                        bot,
                                       const knowledge::hunting_ground& ground,
                                       std::chrono::seconds             slice)
{
    auto distance = [&bot](const point<uint16_t>& position) {
        return std::abs(position.x - bot.position().x) + std::abs(position.y - bot.position().y);
    };

    if (bot.map() != ground.map)
        co_return result::FAILED;

    auto started = clock::now();
    auto seen    = clock::now();
    while (this->alive(bot) && this->_interrupted == false && bot.map() == ground.map)
    {
        auto now = clock::now();
        if (now - started >= slice)
            break;

        if (now - seen >= FARM_IDLE)
        {
            this->_avoided_grounds.insert_or_assign(ground.map, now + GROUND_AVOID);
            this->_ground.reset();
            co_return result::FAILED;
        }

        auto keys    = this->experience_keys(bot);
        auto trapped = this->_hazards.tiles(ground.map);
        auto max_hp  = std::max(bot.base_hp(), bot.hp());
        auto explore = bot.hp() >= max_hp * EXPLORE_HP_RATIO &&
                       std::uniform_real_distribution<double>(0.0, 1.0)(this->_random) < this->_persona.explore_rate;
        auto full = std::unordered_set<uint16_t>{};
        for (auto& [_, owned] : bot.items())
        {
            auto model = table::item->name2item(brain::stack_name(owned.name));
            if (model != nullptr && owned.count >= std::max<uint32_t>(model->capacity, 1))
                full.insert(static_cast<uint16_t>(model->look + ITEM_LOOK_MIN));
        }

        auto item   = std::optional<game_bot::sight::entry>{};
        auto target = std::optional<game_bot::sight::entry>{};
        for (auto& [oid, entry] : bot.sight.entries())
        {
            auto avoided = this->_avoided_targets.find(oid);
            if (avoided != this->_avoided_targets.end() && now < avoided->second)
                continue;

            if (entry.look > ITEM_LOOK_MIN)
            {
                auto unpickable = this->_unpickable.find(entry.look);
                if ((unpickable != this->_unpickable.end() && now < unpickable->second) ||
                    this->_knowledge.warp_at(bot.map(), entry.position) ||
                    trapped.contains(static_cast<uint32_t>((entry.position.y << 16) | entry.position.x)))
                    continue;

                if (full.contains(entry.look) == false && distance(entry.position) <= ITEM_LOOT_RANGE &&
                    (item.has_value() == false || distance(entry.position) < distance(item->position)) &&
                    this->claims(bot, entry.position))
                    item = entry;
            }
            else
            {
                auto prey = this->identify(bot, ground, entry);
                if (prey == nullptr)
                    continue;

                if (this->dangerous(bot, *prey, keys))
                {
                    auto record = this->_experience.find(keys, prey->name);
                    if (explore == false || (record.has_value() && record->fights >= EXPLORE_FIGHTS))
                        continue;
                }

                if (target.has_value() == false || distance(entry.position) < distance(target->position))
                    target = entry;
            }
        }

        auto outcome = result::SUCCEEDED;
        if (item.has_value())
        {
            outcome = co_await this->walk(bot, item->position, 0);
            if (outcome == result::SUCCEEDED)
            {
                auto since  = clock::now();
                auto before = bot.items();
                auto oid    = item->oid;
                bot.send(*bot.make<game_reqs::loot>(true));
                auto reason = co_await this->expect(
                    bot,
                    since,
                    {"HOLD_FULL", "INVENTORY_FULL", "DROP_WARMTH"},
                    [&bot, &before, oid]() {
                        return bot.sight.entries().contains(oid) == false || bot.items().size() != before.size() ||
                               std::equal(before.begin(),
                                          before.end(),
                                          bot.items().begin(),
                                          [](const auto& lhs, const auto& rhs) {
                                              return lhs.first == rhs.first && lhs.second.count == rhs.second.count;
                                          }) == false;
                    },
                    ACTION_TIMEOUT);

                auto detail    = Json::Value{};
                detail["look"] = item->look;
                detail["kept"] =
                    bot.items().size() != before.size() ||
                    std::equal(before.begin(), before.end(), bot.items().begin(), [](const auto& lhs, const auto& rhs) {
                        return lhs.first == rhs.first && lhs.second.count == rhs.second.count;
                    }) == false;
                detail["group"] = this->_grouped;
                detail["left"]  = static_cast<uint32_t>(
                    std::count_if(bot.sight.entries().begin(), bot.sight.entries().end(), [&item](const auto& pair) {
                        return pair.second.look > ITEM_LOOK_MIN && pair.second.position == item->position;
                    }));
                if (reason == "TIMEOUT")
                {
                    auto believed = bot.position();
                    bot.send(*bot.make<game_reqs::screen_refresh>());
                    co_await bot.thread()->sleep(LOOT_RESYNC_WAIT);
                    detail["believed"] = Json::Value{Json::arrayValue};
                    detail["believed"].append(believed.x);
                    detail["believed"].append(believed.y);
                    detail["server"] = Json::Value{Json::arrayValue};
                    detail["server"].append(bot.position().x);
                    detail["server"].append(bot.position().y);
                    if (bot.position() == believed)
                    {
                        for (auto& [left, entry] : bot.sight.entries())
                        {
                            if (entry.look > ITEM_LOOK_MIN && entry.position == item->position)
                                this->_avoided_targets.insert_or_assign(left, clock::now() + TARGET_AVOID);
                        }
                    }
                }
                this->report(bot, "loot", reason, detail);

                if (reason == "HOLD_FULL")
                    this->_unpickable.insert_or_assign(item->look, clock::now() + UNPICKABLE_AVOID);
                else if (reason == "INVENTORY_FULL")
                    this->_sell_urgent = true;
                else if (reason == "DROP_WARMTH")
                {
                    for (auto& [other, entry] : bot.sight.entries())
                    {
                        if (entry.look > ITEM_LOOK_MIN && entry.position == item->position)
                            this->_avoided_targets.insert_or_assign(other, clock::now() + WARMTH_AVOID);
                    }
                    this->_avoided_targets.insert_or_assign(oid, clock::now() + WARMTH_AVOID);
                }
            }

            if (outcome != result::INTERRUPTED && bot.sight.entries().contains(item->oid))
                this->_avoided_targets.insert_or_assign(item->oid, clock::now() + TARGET_AVOID);
        }
        else if (target.has_value())
        {
            seen    = clock::now();
            outcome = co_await this->fight(bot, target->oid);
        }
        else
        {
            auto areas = std::vector<std::pair<point<uint16_t>, point<uint16_t>>>{};
            for (auto& prey : ground.preys)
            {
                if (this->dangerous(bot, prey, keys) == false)
                    areas.insert(areas.end(), prey.areas.begin(), prey.areas.end());
            }

            auto patrol = ground.center;
            if (areas.empty() == false)
            {
                auto& [begin, end] =
                    areas.at(std::uniform_int_distribution<size_t>(0, areas.size() - 1)(this->_random));
                patrol = point<uint16_t>(std::uniform_int_distribution<int>(std::min(begin.x, end.x),
                                                                            std::max(begin.x, end.x))(this->_random),
                                         std::uniform_int_distribution<int>(std::min(begin.y, end.y),
                                                                            std::max(begin.y, end.y))(this->_random));
            }
            outcome = co_await this->walk(bot, patrol, 2);
            if (outcome == result::FAILED)
                co_await bot.thread()->sleep(this->paced(1000ms));
            else if (outcome == result::SUCCEEDED)
                co_await bot.thread()->sleep(this->_persona.step_delay);
        }

        if (outcome == result::INTERRUPTED)
            co_return result::INTERRUPTED;
    }

    if (this->alive(bot) == false || this->_interrupted)
        co_return result::INTERRUPTED;

    co_return result::SUCCEEDED;
}

async::task<brain::result> brain::wander(game_bot& bot)
{
    static const auto directions =
        std::vector<DIRECTION>{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};

    auto direction = directions.at(std::uniform_int_distribution<size_t>(0, directions.size() - 1)(this->_random));
    auto steps     = std::uniform_int_distribution<int>(1, 4)(this->_random);
    for (auto i = 0; i < steps; i++)
    {
        if (this->alive(bot) == false || this->_interrupted)
            co_return result::INTERRUPTED;

        auto next = bot.position();
        switch (direction)
        {
        case DIRECTION::LEFT:
            next.x--;
            break;

        case DIRECTION::TOP:
            next.y--;
            break;

        case DIRECTION::RIGHT:
            next.x++;
            break;

        case DIRECTION::BOTTOM:
            next.y++;
            break;
        }

        auto grid = this->_knowledge.find_grid(bot.map());
        if (grid == nullptr || grid->passable(bot.position(), direction) == false ||
            this->_knowledge.warp_at(bot.map(), next))
            break;

        co_await bot.move(direction, 1, this->_persona.step_delay);
    }

    co_await bot.thread()->sleep(
        this->paced(std::chrono::milliseconds(std::uniform_int_distribution<int>(1000, 3000)(this->_random))));
    co_return result::SUCCEEDED;
}

async::task<brain::result> brain::travel(game_bot& bot, uint16_t map, const point<uint16_t>& target, uint16_t range)
{
    auto model = bot.map() != map ? table::map->find(bot.map()) : nullptr;
    auto root  = model != nullptr ? table::map->find(model->root) : nullptr;
    if (root != nullptr && root->cardinal.empty() == false && bot.state() != STATE::GHOST &&
        bot.get_spell_slot_by_name("비영사천문") != 0xFF && bot.mp() >= RETURN_MP)
    {
        auto current = this->_knowledge.distances(bot.map(), bot.position(), bot.level());
        auto found   = current.find(map);
        auto best =
            found != current.end() ? static_cast<double>(found->second) : std::numeric_limits<double>::infinity();
        auto input = std::string{};
        for (auto& [direction, dsl] : root->cardinal)
        {
            auto gate   = fb::model::dsl::map(dsl.params);
            auto center = point<uint16_t>((gate.x + gate.right) / 2, (gate.y + gate.bottom) / 2);
            auto steps  = std::numeric_limits<double>::infinity();
            if (gate.id == map)
            {
                steps = std::abs(center.x - target.x) + std::abs(center.y - target.y);
            }
            else
            {
                auto distances = this->_knowledge.distances(static_cast<uint16_t>(gate.id), center, bot.level());
                auto reached   = distances.find(map);
                if (reached != distances.end())
                    steps = reached->second;
            }

            if (steps + CARDINAL_SAVING < best)
            {
                best  = steps;
                input = direction == CARDINAL_DIRECTION::EAST    ? "동"
                        : direction == CARDINAL_DIRECTION::WEST  ? "서"
                        : direction == CARDINAL_DIRECTION::SOUTH ? "남"
                                                                 : "북";
            }
        }

        if (input.empty() == false)
            std::ignore = co_await this->cast(bot, "비영사천문", input, 0);
    }

    auto detail    = Json::Value{};
    detail["to"]   = map;
    detail["from"] = bot.map();
    if (bot.map() != map)
    {
        auto route = this->_knowledge.route(bot.map(), bot.position(), map, bot.level());
        if (route.has_value() == false)
        {
            this->report(bot, "travel", "NO_ROUTE", detail);
            co_return result::FAILED;
        }

        for (auto& hop : route.value())
        {
            if (bot.map() != hop.map)
            {
                detail["expected"] = hop.map;
                this->report(bot, "travel", "OFF_ROUTE", detail);
                co_return result::FAILED;
            }

            auto dest   = hop.dest;
            auto reason = std::string{};
            for (auto attempt = 0u;; attempt++)
            {
                auto arrived = co_await this->walk(bot, hop.before, 1);
                if (arrived == result::FAILED)
                {
                    detail["gate"] = Json::Value{Json::arrayValue};
                    detail["gate"].append(hop.before.x);
                    detail["gate"].append(hop.before.y);
                    this->report(bot, "travel", "GATE_BLOCKED", detail);
                }
                if (arrived != result::SUCCEEDED)
                    co_return arrived;

                auto position  = bot.position();
                auto direction = DIRECTION::BOTTOM;
                if (hop.before.x < position.x)
                    direction = DIRECTION::LEFT;
                else if (hop.before.x > position.x)
                    direction = DIRECTION::RIGHT;
                else if (hop.before.y < position.y)
                    direction = DIRECTION::TOP;
                auto since = clock::now();
                co_await bot.move(direction, 1, this->_persona.step_delay);
                if (hop.world != knowledge::NO_WORLD)
                    bot.send(*bot.make<game_reqs::map_world>(hop.world, hop.world_from, hop.world_to));

                reason = co_await this->expect(
                    bot,
                    since,
                    {"WARP_DENIED", "MOVE_DENIED", "GHOST_FORBIDDEN"},
                    [&bot, dest]() {
                        return bot.map() == dest;
                    },
                    std::chrono::duration_cast<std::chrono::milliseconds>(WARP_TIMEOUT));

                if (reason == "INTERRUPTED")
                    co_return result::INTERRUPTED;

                if (bot.map() == dest || bot.map() != hop.map || reason != "TIMEOUT" || attempt >= WARP_RETRIES)
                    break;

                bot.send(*bot.make<game_reqs::screen_refresh>());
                co_await bot.thread()->sleep(LOOT_RESYNC_WAIT);
            }

            if (reason.empty() == false)
            {
                auto detail      = Json::Value{};
                detail["dest"]   = hop.dest;
                detail["before"] = Json::Value{Json::arrayValue};
                detail["before"].append(hop.before.x);
                detail["before"].append(hop.before.y);
                this->report(bot, "warp", reason, detail);
            }

            if (hop.world != knowledge::NO_WORLD)
                fb::logger::debug("load brain: bot={} world {}:{}->{} map={} arrived={}",
                                  bot.name(),
                                  hop.world,
                                  hop.world_from,
                                  hop.world_to,
                                  hop.dest,
                                  bot.map() == hop.dest);

            if (bot.map() != hop.dest)
            {
                detail["landed"] = bot.map();
                detail["dest"]   = hop.dest;
                this->report(bot, "travel", "WARP_MISSED", detail);
                co_return result::FAILED;
            }
        }
    }

    auto walked = co_await this->walk(bot, target, range);
    if (walked == result::FAILED)
        this->report(bot, "travel", "TARGET_BLOCKED", detail);
    co_return walked;
}

async::task<brain::result> brain::walk(game_bot& bot, const point<uint16_t>& target, uint16_t range)
{
    auto map     = bot.map();
    auto blocked = 0;
    auto settled = this->_persona.step_delay >= MOVE_SETTLE;
    auto trail   = std::unordered_set<uint32_t>{};
    while (blocked < WALK_REPLAN_LIMIT)
    {
        if (this->alive(bot) == false || this->_interrupted)
            co_return result::INTERRUPTED;

        auto position = bot.position();
        if (std::abs(position.x - target.x) + std::abs(position.y - target.y) <= range)
        {
            if (settled)
                co_return result::SUCCEEDED;

            co_await bot.thread()->sleep(MOVE_SETTLE - this->_persona.step_delay);
            settled = true;
            continue;
        }

        auto& learned  = this->_learned_blocks[map];
        auto  occupied = std::unordered_set<uint32_t>{};
        auto  mobs     = std::unordered_map<uint32_t, const game_bot::sight::entry*>{};
        for (auto& [_, entry] : bot.sight.entries())
        {
            if (entry.look > ITEM_LOOK_MIN)
                continue;

            auto key = static_cast<uint32_t>((entry.position.y << 16) | entry.position.x);
            occupied.insert(key);
            if (entry.look > knowledge::MOB_LOOK_OFFSET && this->_knowledge.npc_at(map, entry.position) == false)
                mobs.insert_or_assign(key, &entry);
        }

        auto now     = clock::now();
        auto crowded = true;
        auto trapped = this->_hazards.tiles(map);
        auto closed  = [&](const point<uint16_t>& next) {
            auto key   = static_cast<uint32_t>((next.y << 16) | next.x);
            auto found = learned.find(key);
            return (found != learned.end() && now < found->second) || (crowded && occupied.contains(key)) ||
                   (trapped.contains(key) && next != target);
        };
        auto path   = this->_knowledge.path(map, position, target, range, closed);
        crowded     = false;
        auto direct = this->_knowledge.path(map, position, target, range, closed);

        auto blocker = static_cast<const game_bot::sight::entry*>(nullptr);
        if (this->_clearing == false && bot.state() != STATE::GHOST && direct.has_value())
        {
            auto tile = position;
            for (auto direction : direct.value())
            {
                if (direction == DIRECTION::LEFT)
                    tile.x--;
                else if (direction == DIRECTION::RIGHT)
                    tile.x++;
                else if (direction == DIRECTION::TOP)
                    tile.y--;
                else
                    tile.y++;

                auto found = mobs.find(static_cast<uint32_t>((tile.y << 16) | tile.x));
                if (found != mobs.end())
                {
                    auto avoided = this->_avoided_targets.find(found->second->oid);
                    if (avoided == this->_avoided_targets.end() || now >= avoided->second)
                        blocker = found->second;
                    break;
                }
            }
        }

        if (blocker != nullptr)
        {
            auto keys   = this->experience_keys(bot);
            auto prey   = this->find_prey(map, blocker->look, blocker->oid);
            auto record = prey != nullptr ? this->_experience.find(keys, prey->name) : std::nullopt;

            auto detour = path.has_value() ? static_cast<double>(path->size()) - static_cast<double>(direct->size())
                                           : std::numeric_limits<double>::infinity();
            auto kill   = record.has_value()
                              ? record->seconds * 1000.0 / this->_persona.step_delay.count() + CLEAR_KILL_STEPS
                              : std::numeric_limits<double>::infinity();
            auto worth  = prey != nullptr && this->dangerous(bot, *prey, keys) == false &&
                         (kill < detour || (path.has_value() == false && record.has_value() == false));
            if (worth)
            {
                auto detail      = Json::Value{};
                detail["look"]   = blocker->look;
                detail["detour"] = std::isinf(detour) ? -1.0 : detour;
                detail["kill"]   = std::isinf(kill) ? -1.0 : kill;
                this->report(bot, "clear", {}, detail);

                auto oid        = blocker->oid;
                this->_clearing = true;
                auto cleared    = co_await this->fight(bot, oid);
                this->_clearing = false;
                if (cleared == result::INTERRUPTED)
                    co_return result::INTERRUPTED;

                if (bot.map() != map)
                    co_return result::FAILED;

                if (cleared != result::SUCCEEDED)
                    blocked++;
                continue;
            }
        }

        if (path.has_value() == false)
            path = direct;

        if (path.has_value() == false)
        {
            fb::logger::debug("load brain: bot={} no path map={} from=({},{}) to=({},{}) learned={}",
                              bot.name(),
                              map,
                              position.x,
                              position.y,
                              target.x,
                              target.y,
                              learned.size());
            co_return result::FAILED;
        }

        auto steps = std::min<size_t>(path->size(), WALK_STEPS_PER_PLAN);
        for (size_t i = 0; i < steps; i++)
        {
            if (this->alive(bot) == false || this->_interrupted)
                co_return result::INTERRUPTED;

            auto direction = path->at(i);
            auto expected  = bot.position();
            switch (direction)
            {
            case DIRECTION::LEFT:
                expected.x--;
                break;

            case DIRECTION::TOP:
                expected.y--;
                break;

            case DIRECTION::RIGHT:
                expected.x++;
                break;

            case DIRECTION::BOTTOM:
                expected.y++;
                break;
            }

            auto before = bot.position();
            trail.insert(static_cast<uint32_t>((before.y << 16) | before.x));
            co_await bot.move(direction, 1, this->_persona.step_delay);
            auto moved  = bot.position();
            auto jumped = bot.map() == map && std::abs(moved.x - before.x) + std::abs(moved.y - before.y) > 1 &&
                          moved != expected &&
                          trail.contains(static_cast<uint32_t>((moved.y << 16) | moved.x)) == false;
            if (bot.map() != map || jumped)
            {
                auto detail     = Json::Value{};
                detail["event"] = "hazard";
                detail["from"]  = map;
                detail["tile"].append(expected.x);
                detail["tile"].append(expected.y);
                detail["added"] = this->_hazards.add(map, expected);
                this->_trace.write(bot.name(), detail);
                co_return result::FAILED;
            }

            if (moved != expected)
            {
                auto key = static_cast<uint32_t>((expected.y << 16) | expected.x);
                if (moved == before && occupied.contains(key) == false)
                    learned.insert_or_assign(key, clock::now() + BLOCK_MEMORY);
                else if (moved != before)
                    co_await bot.thread()->sleep(MOVE_SETTLE);
                blocked++;
                settled = this->_persona.step_delay >= MOVE_SETTLE;
                break;
            }
            blocked = 0;
            settled = this->_persona.step_delay >= MOVE_SETTLE;
        }
    }
    fb::logger::debug("load brain: bot={} walk blocked map={} pos=({},{}) to=({},{})",
                      bot.name(),
                      map,
                      bot.position().x,
                      bot.position().y,
                      target.x,
                      target.y);
    co_return result::FAILED;
}

std::vector<std::string> brain::experience_keys(const game_bot& bot) const
{
    return this->_experience.keys(bot, this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE));
}

async::task<brain::result> brain::fight(game_bot& bot, uint32_t oid)
{
    auto entry = bot.sight.entries().find(oid);
    if (entry == bot.sight.entries().end())
        co_return result::FAILED;

    auto look              = entry->second.look;
    auto opponent          = this->find_prey(bot.map(), look, oid);
    auto engaged           = std::shared_ptr<void>(nullptr, [this](void*) {
        this->_opponent_damage = 0;
    });
    this->_opponent_damage = opponent != nullptr ? opponent->damage : 0;
    auto keys              = this->experience_keys(bot);
    auto max_hp            = static_cast<double>(std::max<uint32_t>(std::max(bot.base_hp(), bot.hp()), 1));
    auto start_hp          = bot.hp();
    auto last_hp           = bot.hp();
    auto exp               = bot.exp();
    auto damage            = 0u;
    auto attacked          = std::optional<time_point>{};
    auto misses            = 0;
    auto dealt             = uint64_t{0};
    this->_fighting        = oid;
    this->_fight_damage    = 0;
    auto finish            = [&](experience::OUTCOME outcome, const std::string& reason) {
        this->_fighting = 0;
        if (attacked.has_value() == false)
            return;

        auto seconds = std::chrono::duration<double>(clock::now() - attacked.value()).count();
        if (this->_fight_damage > 0)
        {
            auto span  = std::max(seconds, std::chrono::duration<double>(this->_persona.attack_delay).count());
            auto rate  = static_cast<double>(this->_fight_damage) / span;
            this->_dps = this->_dps >= 0.0 ? this->_dps + (rate - this->_dps) * DPS_ALPHA : rate;
        }
        auto loss   = start_hp > bot.hp() ? (start_hp - bot.hp()) / max_hp : 0.0;
        auto gained = bot.exp() > exp ? static_cast<double>(bot.exp() - exp) : 0.0;
        auto mob    = opponent != nullptr ? opponent->name : std::format("#{}", look);
        this->_experience.add(keys, experience::fight{mob, outcome, seconds, damage / max_hp, loss, gained});
        auto detail       = Json::Value{};
        detail["look"]    = look;
        detail["outcome"] = outcome == experience::OUTCOME::KILLED    ? "KILLED"
                                       : outcome == experience::OUTCOME::ABORTED ? "ABORTED"
                                                                                 : "DIED";
        detail["seconds"] = seconds;
        detail["dealt"]   = static_cast<Json::UInt64>(dealt);
        detail["taken"]   = damage;
        detail["exp"]     = gained;
        this->report(bot, "fight", reason, detail);
    };

    auto deadline = clock::now() + FIGHT_TIMEOUT;
    while (this->alive(bot) && this->_interrupted == false)
    {
        if (bot.hp() < last_hp)
            damage += last_hp - bot.hp();
        last_hp = bot.hp();

        if (bot.state() == STATE::GHOST)
        {
            finish(experience::OUTCOME::DIED, "DIED");
            co_return result::INTERRUPTED;
        }

        auto found = bot.sight.entries().find(oid);
        if (found == bot.sight.entries().end())
        {
            this->_metrics.kills++;
            finish(experience::OUTCOME::KILLED, {});
            co_return result::SUCCEEDED;
        }

        if (co_await this->conjure(bot, oid))
            continue;

        if (attacked.has_value() && bot.hp() < max_hp * this->_persona.rest_below)
            std::ignore = co_await this->eat(bot);

        if (clock::now() >= deadline || (attacked.has_value() && bot.hp() < max_hp * this->_persona.abort_hp_ratio))
        {
            for (auto& [_, spell] : bot.spells())
            {
                auto effect = this->_spellbook.find(spell.name);
                if (clock::now() < deadline && effect != nullptr && effect->role == spellbook::ROLE::ESCAPE &&
                    effect->weight > 0.0 && bot.mp() >= effect->mp)
                {
                    std::ignore = co_await this->cast(bot, spell.name, {}, oid);
                    break;
                }
            }
            finish(experience::OUTCOME::ABORTED, clock::now() >= deadline ? "TIMEOUT" : "LOW_HP");
            this->_avoided_targets.insert_or_assign(oid, clock::now() + TARGET_AVOID);
            co_return result::FAILED;
        }

        auto target   = found->second.position;
        auto position = bot.position();
        auto dx       = target.x - position.x;
        auto dy       = target.y - position.y;
        if (std::abs(dx) + std::abs(dy) != 1)
        {
            auto approached = co_await this->walk(bot, target, 1);
            if (approached == result::FAILED)
                this->_avoided_targets.insert_or_assign(oid, clock::now() + TARGET_AVOID);

            if (approached != result::SUCCEEDED)
                co_return approached;
        }
        else
        {
            auto direction = DIRECTION::BOTTOM;
            if (dx < 0)
                direction = DIRECTION::LEFT;
            else if (dx > 0)
                direction = DIRECTION::RIGHT;
            else if (dy < 0)
                direction = DIRECTION::TOP;

            bot.send(*bot.make<game_reqs::direction>(direction));
            bot.set_direction(direction);

            if (attacked.has_value() == false)
                attacked = clock::now();

            auto finisher = std::string{};
            for (auto& [_, spell] : bot.spells())
            {
                auto effect = this->_spellbook.find(spell.name);
                if (effect != nullptr && effect->role == spellbook::ROLE::STRIKE && effect->weight > 0.0 &&
                    bot.mp() >= effect->mp)
                    finisher = spell.name;
            }
            auto wait = this->_spell_waits.find(finisher);
            auto recovered =
                this->_grouped ||
                std::any_of(this->_supplies.heals.begin(), this->_supplies.heals.end(), [&bot](const auto& heal) {
                    return bot.has_item_by_name(heal.first);
                });
            auto sturdy = opponent != nullptr && opponent->hp > dealt + this->_dps * FINISHER_HITS;
            if (finisher.empty() == false && bot.get_spell_slot_by_name(finisher) != 0xFF &&
                bot.hp() >= max_hp * FINISHER_HP_RATIO && sturdy && recovered &&
                (wait == this->_spell_waits.end() || clock::now() >= wait->second))
            {
                auto cast = co_await this->cast(bot, finisher, {}, 0);
                this->_spell_waits.insert_or_assign(finisher, clock::now() + SPELL_RETRY);
                if (cast)
                    continue;
            }

            auto since = clock::now();
            bot.send(*bot.make<game_reqs::attack>());
            auto reason = co_await this->expect(
                bot,
                since,
                {"TARGET_INVALID", "GHOST_FORBIDDEN"},
                [this, oid, since]() {
                    auto hit = this->_hits.find(oid);
                    return hit != this->_hits.end() && hit->second.first >= since;
                },
                this->_persona.attack_wait);

            if (reason.empty())
            {
                misses  = 0;
                dealt  += this->_hits.at(oid).second;
            }
            else if (reason == "TIMEOUT")
            {
                misses++;
            }

            if ((reason.empty() == false && reason != "TIMEOUT" && reason != "INTERRUPTED") || misses >= ATTACK_MISSES)
            {
                finish(experience::OUTCOME::ABORTED, reason == "TIMEOUT" ? "NO_HIT" : reason);
                this->_avoided_targets.insert_or_assign(oid, clock::now() + TARGET_AVOID);
                co_return result::FAILED;
            }

            auto next = since + this->_persona.attack_delay;
            if (clock::now() < next)
                co_await bot.thread()->sleep(
                    std::chrono::duration_cast<std::chrono::milliseconds>(next - clock::now()));
        }
    }
    co_return result::INTERRUPTED;
}

async::task<brain::result> brain::talk(game_bot& bot, uint32_t oid, const std::vector<std::string>& picks)
{
    auto dialog     = std::optional<integration::dialog_bot>{};
    auto dialog_ext = std::optional<integration::dialog_ext_bot>{};
    auto seq        = bot.last_dialog(dialog, dialog_ext);
    bot.send(*bot.make<game_reqs::click>(oid, game_reqs::click<fb::protocol::CLIENT_VERSION::v550>::FLAG_OBJECT));

    auto answered = co_await this->answer(bot, seq, picks);
    if (this->alive(bot) == false)
        co_return result::INTERRUPTED;

    co_return answered > 0 ? result::SUCCEEDED : result::FAILED;
}

async::task<bool> brain::cast(game_bot& bot, const std::string& name, const std::string& message, uint32_t oid)
{
    auto slot  = bot.get_spell_slot_by_name(name);
    auto spell = bot.get_spell(slot);
    if (spell.has_value() == false)
        co_return false;

    auto since = clock::now();
    auto map   = bot.map();
    auto hp    = bot.hp();
    auto mp    = bot.mp();
    bot.send(
        *bot.make<game_reqs::spell_cast>(static_cast<SPELL_TYPE>(spell->type), slot, message, oid, bot.position()));
    auto reason = co_await this->expect(
        bot,
        since,
        {"NO_MP",
         "NO_HP",
         "GHOST_FORBIDDEN",
         "HUMAN_FORBIDDEN",
         "SPELL_FAILED",
         "SPELL_DELAY",
         "TARGET_INVALID",
         "SPELL_AREA_DENIED"},
        [&]() {
            return this->heard("SPELL_CAST", since) || bot.map() != map || bot.mp() != mp || bot.hp() != hp;
        },
        ACTION_TIMEOUT);

    auto detail     = Json::Value{};
    detail["spell"] = fb::UTF8(name, fb::PLATFORM::WINDOWS);
    if (message.empty() == false)
        detail["input"] = fb::UTF8(message, fb::PLATFORM::WINDOWS);
    detail["target"] = oid;
    this->report(bot, "cast", reason, detail);
    co_return reason.empty();
}

async::task<bool> brain::conjure(game_bot& bot, uint32_t target)
{
    if (bot.state() == STATE::GHOST)
        co_return false;

    if (co_await this->enchant(bot))
        co_return true;

    auto now    = clock::now();
    auto max_hp = std::max(bot.base_hp(), bot.hp());
    auto max_mp = std::max(bot.base_mp(), bot.mp());
    auto ready  = [&](const std::string& name, const spellbook::effect& effect) {
        auto wait = this->_spell_waits.find(name);
        return bot.mp() >= effect.mp && (wait == this->_spell_waits.end() || now >= wait->second);
    };

    auto leader = uint32_t{0};
    auto hurt   = false;
    auto front  = false;
    auto sturdy = false;
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (this->_leader.empty() == false && entry.name == this->_leader)
        {
            auto vital = this->_vitals.find(entry.oid);
            leader     = entry.oid;
            hurt       = vital != this->_vitals.end() && vital->second < LEADER_HEAL_PERCENT;
        }

        if (entry.oid == target)
        {
            auto prey = this->find_prey(bot.map(), entry.look, entry.oid);
            front  = std::abs(entry.position.x - bot.position().x) + std::abs(entry.position.y - bot.position().y) == 1;
            sturdy = prey != nullptr && prey->hp > std::max(this->_dps, MIN_DPS) * DEBUFF_SECONDS;
        }
    }

    auto thirst = 0u;
    for (auto& [_, spell] : bot.spells())
    {
        auto effect = this->_spellbook.find(spell.name);
        if (effect != nullptr && effect->weight > 0.0 &&
            (effect->role == spellbook::ROLE::HEAL || effect->role == spellbook::ROLE::ATTACK ||
             effect->role == spellbook::ROLE::AREA))
            thirst = std::max(thirst, effect->mp);
    }

    using spell_pick = std::optional<std::pair<std::string, spellbook::effect>>;
    auto heal        = spell_pick{};
    auto attack      = spell_pick{};
    auto cure        = spell_pick{};
    auto emergency   = spell_pick{};
    auto mana        = spell_pick{};
    auto exclusive   = spell_pick{};
    auto buff        = spell_pick{};
    auto ally        = spell_pick{};
    auto debuff      = spell_pick{};
    auto ambush      = spell_pick{};
    auto reserve     = max_mp * MP_RESERVE_RATIO;
    for (auto& [_, spell] : bot.spells())
    {
        auto effect = this->_spellbook.find(spell.name);
        if (effect == nullptr || effect->weight <= 0.0 || ready(spell.name, *effect) == false)
            continue;

        auto pick    = std::pair{spell.name, *effect};
        auto active  = bot.has_buff(spell.name);
        auto marked  = this->_spell_waits.find(std::format("{}#{}", spell.name, target));
        auto renewal = this->_spell_waits.find(std::format("{}@{}", spell.name, leader));
        auto renewed = leader == 0 || (renewal != this->_spell_waits.end() && now < renewal->second);
        switch (effect->role)
        {
        case spellbook::ROLE::HEAL:
            if (heal.has_value() == false || effect->hp > heal->second.hp)
                heal = pick;
            break;

        case spellbook::ROLE::ATTACK:
        case spellbook::ROLE::AREA:
            if (target != 0 && bot.mp() >= effect->mp + reserve &&
                (attack.has_value() == false || effect->damage > attack->second.damage))
                attack = pick;
            break;

        case spellbook::ROLE::CURE:
            if (bot.has_buff(effect->cure))
                cure = pick;
            break;

        case spellbook::ROLE::BUFF:
            if (effect->self == false && (active == false || renewed == false))
                ally = pick;
            else if (active || effect->self == false)
                break;
            else if (effect->trigger == spellbook::TRIGGER::EMERGENCY)
                emergency = pick;
            else if (effect->trigger == spellbook::TRIGGER::MANA)
                mana = pick;
            else if (effect->exclusive && (exclusive.has_value() == false || effect->id > exclusive->second.id))
                exclusive = pick;
            else if (effect->exclusive == false && buff.has_value() == false)
                buff = pick;
            break;

        case spellbook::ROLE::DEBUFF:
            if (target != 0 && sturdy && bot.mp() >= effect->mp + reserve &&
                (marked == this->_spell_waits.end() || now >= marked->second) && debuff.has_value() == false)
                debuff = pick;
            break;

        case spellbook::ROLE::AMBUSH:
            if (front)
                ambush = pick;
            break;

        default:
            break;
        }
    }

    if (exclusive.has_value())
    {
        auto strongest = exclusive;
        for (auto& [_, spell] : bot.spells())
        {
            auto effect = this->_spellbook.find(spell.name);
            if (effect != nullptr && effect->role == spellbook::ROLE::BUFF && effect->exclusive &&
                effect->weight > 0.0 && effect->id > strongest->second.id)
                strongest = std::pair{spell.name, *effect};
        }

        if (bot.has_buff(strongest->first) || strongest->first != exclusive->first)
            exclusive.reset();
    }

    auto name    = std::string{};
    auto oid     = uint32_t{0};
    auto key     = std::string{};
    auto expires = std::chrono::duration<double>(BUFF_CONFIRM);
    auto retry   = std::chrono::duration<double>(SPELL_RETRY);
    auto charge  = bot.get_spell_slot_by_name("공력증강") != 0xFF ? this->_spellbook.find("공력증강") : nullptr;
    auto quench  = this->_spell_waits.find(DRINK_WAIT);
    auto thirsty = (quench == this->_spell_waits.end() || now >= quench->second) &&
                   std::any_of(this->_supplies.drinks.begin(), this->_supplies.drinks.end(), [&bot](const auto& drink) {
                       return bot.has_item_by_name(drink.first);
                   });
    auto low_hp = bot.hp() < max_hp * this->_persona.rest_below;
    auto low_mp = bot.mp() < max_mp * MP_LOW_RATIO;
    auto self   = [&bot](const spellbook::effect& effect) {
        return effect.type == SPELL_TYPE::TARGET ? bot.oid() : 0u;
    };

    if (heal.has_value() && low_hp)
    {
        name = heal->first;
        oid  = self(heal->second);
    }
    else if (emergency.has_value() && low_hp)
    {
        name = emergency->first;
    }
    else if (heal.has_value() && heal->second.type == SPELL_TYPE::TARGET && hurt)
    {
        name = heal->first;
        oid  = leader;
    }
    else if (cure.has_value())
    {
        name = cure->first;
        oid  = self(cure->second);
    }
    else if (charge != nullptr && ready("공력증강", *charge) && low_mp &&
             bot.hp() > max_mp * MP_CHARGE_COST + max_hp * this->_persona.rest_below)
    {
        name    = "공력증강";
        expires = std::chrono::duration<double>(MP_CHARGE_RETRY);
        retry   = std::chrono::duration<double>(MP_CHARGE_RETRY);
    }
    else if (low_mp && bot.mp() < thirst && thirsty && this->caster(bot))
    {
        auto drank = co_await this->drink(bot);
        if (drank == false)
            this->_spell_waits.insert_or_assign(DRINK_WAIT, clock::now() + SPELL_RETRY);
        co_return drank;
    }
    else if (mana.has_value() && low_mp)
    {
        name = mana->first;
    }
    else if (exclusive.has_value())
    {
        name = exclusive->first;
        oid  = self(exclusive->second);
    }
    else if (buff.has_value())
    {
        name = buff->first;
        oid  = self(buff->second);
    }
    else if (ally.has_value() && bot.has_buff(ally->first) == false)
    {
        name = ally->first;
        oid  = bot.oid();
    }
    else if (ally.has_value())
    {
        name    = ally->first;
        oid     = leader;
        key     = std::format("{}@{}", ally->first, leader);
        expires = std::chrono::seconds(ally->second.buff_time) * BUFF_RENEW_RATIO;
    }
    else if (debuff.has_value())
    {
        name    = debuff->first;
        oid     = target;
        key     = std::format("{}#{}", debuff->first, target);
        expires = std::chrono::seconds(debuff->second.buff_time) * DEBUFF_RENEW_RATIO;
    }
    else if (ambush.has_value())
    {
        name  = ambush->first;
        retry = std::chrono::duration<double>(AMBUSH_RETRY);
    }
    else if (attack.has_value())
    {
        name = attack->first;
        oid  = target;
    }

    if (name.empty())
        co_return false;

    auto cast = co_await this->cast(bot, name, {}, oid);
    auto wait = clock::now() + std::chrono::duration_cast<clock::duration>(cast ? expires : retry);
    this->_spell_waits.insert_or_assign(key.empty() ? name : key, wait);
    if (key.empty() == false && cast == false)
        this->_spell_waits.insert_or_assign(name, wait);
    co_return cast;
}

void brain::announce(game_bot& bot)
{
    auto now = clock::now();
    if (this->_grouped == false)
        return;

    if (this->_leader.empty() == false && now - this->_leader_seen > LEADER_TIMEOUT)
    {
        bot.send(*bot.make<game_reqs::group>(bot.name()));
        this->_grouped    = false;
        this->_leader_map = 0xFFFF;
        this->_leader.clear();
    }
    else if (this->_leader.empty() && now - this->_announced > ANNOUNCE_INTERVAL)
    {
        auto map = table::map->find(bot.map());
        if (map != nullptr)
            bot.send(*bot.make<game_reqs::whisper>(
                "!!",
                std::format("위치 {} {} {} {}", map->name, bot.map(), bot.position().x, bot.position().y)));
        this->_announced = now;
    }
}

void brain::advertise(game_bot& bot)
{
    auto now = clock::now();
    if (now - this->_advertised < ADVERTISE_INTERVAL)
        return;
    this->_advertised = now;

    auto needs = std::unordered_map<std::string, uint32_t>{};
    if (bot.state() != STATE::GHOST)
    {
        for (auto& [item, count] : this->reserved(bot))
        {
            if (this->_knowledge.sellers(item).empty() && bot.get_item_count_by_name(item) < count)
                needs.insert({item, count - static_cast<uint32_t>(bot.get_item_count_by_name(item))});
        }
    }

    for (auto i = this->_wanted.begin(); i != this->_wanted.end();)
    {
        if (needs.contains(*i))
        {
            i++;
        }
        else
        {
            this->_market.withdraw(bot.name(), *i);
            i = this->_wanted.erase(i);
        }
    }

    for (auto& [item, count] : needs)
    {
        this->_wanted.insert(item);
        if (this->_market.post(bot.name(), item, count, bot.map(), bot.position()))
        {
            auto text  = std::format("{} {}개 구합니다", item, count);
            auto shout = std::find_if(bot.spells().begin(), bot.spells().end(), [this](const auto& pair) {
                auto effect = this->_spellbook.find(pair.second.name);
                return effect != nullptr && effect->role == spellbook::ROLE::SHOUT && effect->weight > 0.0;
            });
            if (shout != bot.spells().end())
                bot.send(*bot.make<game_reqs::spell_cast>(static_cast<SPELL_TYPE>(shout->second.type),
                                                          shout->first,
                                                          text,
                                                          0,
                                                          bot.position()));
            else
                bot.send(*bot.make<game_reqs::chat>(false, text));

            auto entry     = Json::Value{};
            entry["event"] = "want";
            entry["item"]  = fb::UTF8(item, fb::PLATFORM::WINDOWS);
            entry["count"] = count;
            this->_trace.write(bot.name(), entry);
        }
    }
}

void brain::regroup(game_bot& bot)
{
    auto mate = this->_partner.empty() ? this->_leader : this->_partner;
    if (this->_grouped == false || mate.empty())
        return;

    auto now = clock::now();
    for (auto& [_, entry] : bot.sight.entries())
    {
        if (entry.name == mate)
            this->_partner_seen = now;
    }

    auto level = this->_roster.level(mate);
    auto gap   = level.has_value() ? std::abs(static_cast<int>(level.value()) - static_cast<int>(bot.level())) : 0;

    auto reason = std::string{};
    if (now - this->_partner_seen > this->_job.group_apart)
        reason = "APART";
    else if (gap > this->_job.group_gap)
        reason = "LEVEL_GAP";

    if (reason.empty() == false)
    {
        bot.send(*bot.make<game_reqs::group>(bot.name()));
        this->_group_tries.insert_or_assign(mate, now + GROUP_SHUN);
        this->_grouped    = false;
        this->_leader_map = 0xFFFF;
        this->_leader.clear();
        this->_partner.clear();

        auto entry       = Json::Value{};
        entry["event"]   = "ungroup";
        entry["partner"] = fb::UTF8(mate, fb::PLATFORM::WINDOWS);
        entry["reason"]  = reason;
        entry["gap"]     = gap;
        this->_trace.write(bot.name(), entry);
    }
}

bool brain::claims(const game_bot& bot, const point<uint16_t>& position)
{
    auto mate = this->_partner.empty() ? this->_leader : this->_partner;
    if (this->_grouped == false || mate.empty())
        return true;

    auto first  = std::numeric_limits<uint32_t>::max();
    auto beside = false;
    for (auto& [oid, entry] : bot.sight.entries())
    {
        if (entry.look > ITEM_LOOK_MIN && entry.position == position)
            first = std::min(first, oid);
        else if (entry.name == mate)
            beside = true;
    }

    if (first == std::numeric_limits<uint32_t>::max())
        return false;

    auto now     = clock::now();
    auto sighted = this->_piles.try_emplace(first, now).first->second;
    if (this->_piles.size() > PILES_LIMIT)
    {
        std::erase_if(this->_piles, [&bot](const auto& pile) {
            return bot.sight.entries().contains(pile.first) == false;
        });
    }

    auto turn = this->_partner.empty() ? 1u : 0u;
    return first % 2 == turn || beside == false || now - sighted > LOOT_YIELD;
}

const questbook::quest* brain::choose_quest(const game_bot& bot) const
{
    auto now = clock::now();
    if (this->_quest_active != 0)
    {
        auto active =
            std::find_if(this->_questbook.quests.begin(), this->_questbook.quests.end(), [this](const auto& quest) {
                return quest.id == this->_quest_active;
            });
        if (active != this->_questbook.quests.end())
            return &*active;
    }

    if (now < this->_quest_checked + this->paced(QUEST_RECHECK))
        return this->_quest_choice;

    this->_quest_checked = now;
    this->_quest_choice  = nullptr;
    auto why             = Json::Value{Json::objectValue};
    auto publish         = [&](const questbook::quest* choice) {
        auto entry      = Json::Value{};
        entry["event"]  = "quest_pick";
        entry["choice"] = choice != nullptr ? choice->id : 0u;
        entry["why"]    = why;
        auto signature  = Json::FastWriter{}.write(entry);
        if (signature == this->_quest_why)
            return;

        this->_quest_why = signature;
        this->_trace.write(bot.name(), entry);
    };
    auto visits  = this->_knowledge.search(bot.map(), bot.position(), bot.level());
    auto grounds = std::unordered_map<uint16_t, const knowledge::hunting_ground*>{};
    for (auto& ground : this->_knowledge.hunting_grounds())
    {
        grounds.insert({ground.map, &ground});
    }

    auto keys   = this->experience_keys(bot);
    auto risks  = std::unordered_map<uint16_t, bool>{};
    auto unsafe = [&](uint16_t map) {
        auto known = risks.find(map);
        if (known != risks.end())
            return known->second;

        auto ground = grounds.find(map);
        auto hurt   = this->_hurt_maps.find(map);
        auto risky  = (hurt != this->_hurt_maps.end() && now < hurt->second) ||
                     (ground != grounds.end() &&
                      std::any_of(ground->second->preys.begin(), ground->second->preys.end(), [&](const auto& prey) {
                          return prey.aggressive && this->harmful(bot, prey, keys);
                      }));
        risks.insert({map, risky});
        return risky;
    };
    if (unsafe(bot.map()))
    {
        why["all"] = "HOSTILE";
        publish(nullptr);
        return nullptr;
    }
    auto route = [&](uint16_t map) {
        auto at = map;
        for (auto hop = 0u; hop < QUEST_HOPS; hop++)
        {
            auto visit = visits.find(at);
            if (visit == visits.end() || visit->second.cost > QUEST_STEPS || (at != bot.map() && unsafe(at)))
                return false;
            if (visit->second.via == nullptr)
                return true;
            at = visit->second.via->map;
        }
        return false;
    };
    auto blocker = [&](uint16_t map) {
        auto at = map;
        for (auto hop = 0u; hop < QUEST_HOPS; hop++)
        {
            auto visit = visits.find(at);
            if (visit == visits.end())
                return std::string{"UNREACHABLE "} + std::to_string(at);
            if (visit->second.cost > QUEST_STEPS)
                return std::string{"COST"};
            if (at != bot.map() && unsafe(at))
                return std::string{"UNSAFE "} + std::to_string(at);
            if (visit->second.via == nullptr)
                return std::string{"OK"};
            at = visit->second.via->map;
        }
        return std::string{"HOPS"};
    };

    auto items = table::item;
    auto worth = [&](const std::string& name, uint32_t count) {
        auto model = items->name2item(name);
        auto price = std::max<uint64_t>(this->_knowledge.price(name), model != nullptr ? model->price : 0);
        return static_cast<double>(price) * count;
    };

    auto best = static_cast<const questbook::quest*>(nullptr);
    auto gain = 0.0;
    for (auto& quest : this->_questbook.quests)
    {
        auto shun = this->_quest_shun.find(quest.id);
        auto key  = std::to_string(quest.id);
        if (this->_quests_done.contains(quest.id))
        {
            why[key] = "DONE";
            continue;
        }
        if (shun != this->_quest_shun.end() && now < shun->second)
        {
            why[key] = "SHUN";
            continue;
        }
        if (bot.level() < quest.level)
        {
            why[key] = "LEVEL";
            continue;
        }
        if (bot.money() < quest.money)
        {
            why[key] = "MONEY";
            continue;
        }

        auto spots     = this->_knowledge.npc_spots(quest.npcs.front());
        auto reachable = std::any_of(spots.begin(), spots.end(), [&route](const auto& spot) {
            return route(spot.map);
        });
        if (reachable == false)
        {
            why[key] = spots.empty() ? std::string{"NO_SPOT"} : blocker(spots.front().map);
            continue;
        }

        auto obtainable = std::all_of(quest.consumes.begin(), quest.consumes.end(), [&](const auto& consume) {
            return bot.get_item_count_by_name(consume.first) >= consume.second ||
                   this->_knowledge.sellers(consume.first).empty() == false ||
                   this->_knowledge.droppable(consume.first);
        });
        if (obtainable == false)
        {
            why[key] = "MATERIAL";
            continue;
        }

        auto value = static_cast<double>(quest.reward_money) + quest.reward_exp * QUEST_EXP_VALUE;
        for (auto& [name, count] : quest.rewards)
        {
            value += worth(name, count);
        }

        auto cost = static_cast<double>(quest.money);
        for (auto& [name, count] : quest.consumes)
        {
            cost += worth(name, count);
        }

        why[key] = value - cost;
        if (value - cost >= 0.0 && (best == nullptr || value - cost > gain))
        {
            gain = value - cost;
            best = &quest;
        }
    }

    this->_quest_checked = now;
    this->_quest_choice  = best;
    publish(best);
    return best;
}

async::task<brain::result> brain::quest(game_bot& bot)
{
    auto quest = this->choose_quest(bot);
    if (quest == nullptr)
        co_return result::FAILED;

    this->_quest_active = quest->id;

    auto detail  = Json::Value{};
    detail["id"] = quest->id;
    for (auto& [item, count] : quest->consumes)
    {
        if (bot.get_item_count_by_name(item) >= count)
            continue;

        auto bought = co_await this->buy(bot, item, count);
        if (bought == result::INTERRUPTED)
            co_return bought;

        if (bot.get_item_count_by_name(item) < count)
        {
            detail["material"] = fb::UTF8(item, fb::PLATFORM::WINDOWS);
            this->report(bot, "quest", "MATERIAL_SHORT", detail);
            this->_cooldowns[static_cast<size_t>(GOAL::QUEST)] = clock::now() + SHOP_AVOID;
            this->_quest_active                                = 0;
            co_return result::FAILED;
        }
    }

    auto before = this->owned(bot);
    auto money  = bot.money();
    auto moved  = [&]() {
        auto after  = this->owned(bot);
        auto gained = std::any_of(quest->rewards.begin(), quest->rewards.end(), [&](const auto& reward) {
            auto was = before.find(reward.first);
            auto now = after.find(reward.first);
            return (now != after.end() ? now->second : 0u) > (was != before.end() ? was->second : 0u);
        });
        auto spent  = std::any_of(quest->consumes.begin(), quest->consumes.end(), [&](const auto& consume) {
            auto was = before.find(consume.first);
            auto now = after.find(consume.first);
            return (now != after.end() ? now->second : 0u) < (was != before.end() ? was->second : 0u);
        });
        return gained || spent || bot.money() > money;
    };

    auto talked = 0u;
    for (auto npc : quest->npcs)
    {
        auto spot = this->nearest(bot, this->_knowledge.npc_spots(npc));
        if (spot.has_value() == false)
            continue;

        auto arrived = co_await this->travel(bot, spot->map, spot->position, TALK_RANGE);
        if (arrived == result::INTERRUPTED)
        {
            auto hurt =
                bot.state() == STATE::GHOST || bot.hp() < std::max(bot.base_hp(), bot.hp()) * this->_persona.rest_below;
            if (hurt && ++this->_quest_stops[quest->id] >= QUEST_STOPS)
            {
                this->_quest_shun.insert_or_assign(quest->id, clock::now() + QUEST_SHUN);
                this->_quest_stops.erase(quest->id);
                this->_quest_checked = time_point{};
                this->_quest_active  = 0;
                this->report(bot, "quest", "DANGER", detail);
            }
            co_return arrived;
        }
        else if (arrived != result::SUCCEEDED)
        {
            continue;
        }

        auto oid = uint32_t{0};
        for (auto& [_, entry] : bot.sight.entries())
        {
            if (entry.position == spot->position)
                oid = entry.oid;
        }
        if (oid == 0)
            continue;

        for (auto attempt = 0u; attempt < QUEST_TALKS && moved() == false; attempt++)
        {
            auto said = co_await this->talk(bot, oid, quest->picks);
            if (said == result::INTERRUPTED)
                co_return said;
            talked++;
        }

        if (moved())
            break;
    }

    auto progressed  = moved();
    detail["npcs"]   = talked;
    detail["reward"] = progressed;
    this->report(bot, "quest", progressed ? std::string{} : std::string{"NO_PROGRESS"}, detail);

    if (progressed)
        this->_quests_done.insert(quest->id);
    else
        this->_quest_shun.insert_or_assign(quest->id, clock::now() + QUEST_SHUN);
    this->_quest_checked = time_point{};
    this->_quest_active  = 0;
    co_return progressed ? result::SUCCEEDED : result::FAILED;
}

bool brain::surplus(const game_bot& bot, const market::want& want, uint32_t need) const
{
    auto model = table::item->name2item(want.item);
    if (model == nullptr || model->trade == false)
        return false;

    auto owned = static_cast<uint32_t>(bot.get_item_count_by_name(want.item));
    auto keep  = this->held(bot, want.item, this->reserved(bot));
    return owned > keep && owned - keep >= need;
}

async::task<brain::result> brain::trade(game_bot& bot)
{
    auto want = this->_market.claim(bot.name(), [this, &bot](const auto& want) {
        return this->surplus(bot, want, std::min(want.count, TRADE_BATCH));
    });
    if (want.has_value() == false)
        co_return result::FAILED;

    bot.send(*bot.make<game_reqs::whisper>(want->name, std::format("{} 드릴게요", want->item)));

    auto given  = 0u;
    auto reason = std::string{};
    for (auto i = 0; i < TRADE_SEARCHES && given == 0; i++)
    {
        auto latest = this->_market.find(want->name, want->item);
        if (latest.has_value() == false)
            break;

        auto arrived = co_await this->travel(bot, latest->map, latest->position, FOLLOW_RANGE);
        if (arrived == result::INTERRUPTED)
        {
            this->_market.release(bot.name());
            co_return arrived;
        }
        else if (arrived != result::SUCCEEDED)
        {
            break;
        }

        auto oid = uint32_t{0};
        for (auto& [_, entry] : bot.sight.entries())
        {
            if (entry.name == want->name)
                oid = entry.oid;
        }
        if (oid == 0)
            continue;

        auto position   = bot.sight.entries().at(oid).position;
        auto approached = co_await this->walk(bot, position, 1);
        if (approached == result::INTERRUPTED)
        {
            this->_market.release(bot.name());
            co_return approached;
        }

        auto found = bot.sight.entries().find(oid);
        if (found == bot.sight.entries().end())
            continue;

        auto dx = found->second.position.x - bot.position().x;
        auto dy = found->second.position.y - bot.position().y;
        if (std::abs(dx) + std::abs(dy) != 1)
            continue;

        auto direction = DIRECTION::BOTTOM;
        if (dx < 0)
            direction = DIRECTION::LEFT;
        else if (dx > 0)
            direction = DIRECTION::RIGHT;
        else if (dy < 0)
            direction = DIRECTION::TOP;
        bot.send(*bot.make<game_reqs::direction>(direction));
        bot.set_direction(direction);

        while (given < latest->count && this->surplus(bot, *latest, 1))
        {
            auto slot  = bot.get_item_slot_by_name(want->item);
            auto count = bot.get_item_count_by_name(want->item);
            auto since = clock::now();
            bot.send(*bot.make<game_reqs::give_item>(slot, false));
            reason = co_await this->expect(
                bot,
                since,
                {"GHOST_FORBIDDEN", "RIDING_FORBIDDEN"},
                [&]() {
                    return bot.get_item_count_by_name(want->item) != count;
                },
                ACTION_TIMEOUT);
            if (reason.empty() == false)
                break;
            given++;
        }
        break;
    }

    this->_market.release(bot.name());

    auto detail     = Json::Value{};
    detail["item"]  = fb::UTF8(want->item, fb::PLATFORM::WINDOWS);
    detail["to"]    = fb::UTF8(want->name, fb::PLATFORM::WINDOWS);
    detail["given"] = given;
    this->report(bot, "trade", given > 0 ? std::string{} : (reason.empty() ? std::string{"NOT_MET"} : reason), detail);

    if (reason == "INTERRUPTED")
        co_return result::INTERRUPTED;
    if (given == 0)
        this->_cooldowns[static_cast<size_t>(GOAL::TRADE)] = clock::now() + TRADE_COOLDOWN;
    co_return given > 0 ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::group(game_bot& bot)
{
    if (this->_group_ready == false)
    {
        auto since = clock::now();
        bot.send(*bot.make<game_reqs::group>(bot.name()));
        auto reason = co_await this->expect(
            bot,
            since,
            {"GROUP_DISABLED"},
            [&]() {
                return this->heard("GROUP_SELF", since);
            },
            ACTION_TIMEOUT);
        if (reason == "GROUP_DISABLED")
            bot.send(*bot.make<game_reqs::update_option>(OPTION::GROUP, false));

        this->_group_ready = reason.empty() || reason == "GROUP_DISABLED";
        co_return this->_group_ready ? result::SUCCEEDED : result::FAILED;
    }

    auto users = std::vector<game_bot::user_data>{};
    auto seq   = bot.users(users);
    bot.send(*bot.make<game_reqs::user_list>());
    auto deadline = clock::now() + DIALOG_TIMEOUT;
    while (this->alive(bot) && bot.users(users) == seq && clock::now() < deadline)
    {
        co_await bot.thread()->sleep(100ms);
    }

    auto now     = clock::now();
    auto partner = std::optional<std::string>{};
    auto closest = this->_job.group_gap + 1;
    for (auto& user : users)
    {
        auto tried = this->_group_tries.find(user.name);
        auto level = this->_roster.level(user.name);
        auto gap   = level.has_value() ? std::abs(static_cast<int>(level.value()) - static_cast<int>(bot.level()))
                                       : this->_job.group_gap + 1;
        if (user.cls == static_cast<uint8_t>(CLASS::POET) && user.name != bot.name() && gap < closest &&
            (tried == this->_group_tries.end() || now >= tried->second))
        {
            partner = user.name;
            closest = gap;
        }
    }

    if (partner.has_value() == false)
    {
        this->_cooldowns[static_cast<size_t>(GOAL::GROUP)] = now + GROUP_RETRY;
        co_return result::FAILED;
    }

    this->_group_tries.insert_or_assign(partner.value(), now + GROUP_RETRY);
    auto since = clock::now();
    bot.send(*bot.make<game_reqs::group>(partner.value()));
    auto reason = co_await this->expect(
        bot,
        since,
        {"GROUP_TARGET_DISABLED", "GROUP_DENIED", "GROUP_DISABLED"},
        [&]() {
            return this->heard("GROUP_JOINED", since);
        },
        ACTION_TIMEOUT);

    if (reason.empty())
    {
        this->_grouped      = true;
        this->_partner      = partner.value();
        this->_partner_seen = clock::now();
        this->_leader.clear();
        this->_announced = time_point{};
    }
    else if (reason == "GROUP_DISABLED")
    {
        this->_group_ready = false;
    }

    auto detail       = Json::Value{};
    detail["partner"] = fb::UTF8(partner.value(), fb::PLATFORM::WINDOWS);
    this->report(bot, "group", reason, detail);
    co_return reason.empty() ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::follow(game_bot& bot)
{
    auto distance =
        std::abs(bot.position().x - this->_leader_position.x) + std::abs(bot.position().y - this->_leader_position.y);
    if (bot.map() != this->_leader_map || distance > FOLLOW_RANGE * 2)
    {
        for (auto& [_, spell] : bot.spells())
        {
            auto effect = this->_spellbook.find(spell.name);
            auto wait   = this->_spell_waits.find(spell.name);
            if (bot.map() != this->_leader_map && effect != nullptr && effect->role == spellbook::ROLE::GOTO &&
                effect->weight > 0.0 && bot.mp() >= effect->mp &&
                (wait == this->_spell_waits.end() || clock::now() >= wait->second))
            {
                auto name = spell.name;
                auto map  = bot.map();
                auto cast = co_await this->cast(bot, name, this->_leader, 0);
                this->_spell_waits.insert_or_assign(name, clock::now() + (cast ? SPELL_RETRY : GROUP_RETRY));
                if (bot.map() != map)
                    co_return result::SUCCEEDED;
                break;
            }
        }

        auto arrived = co_await this->travel(bot, this->_leader_map, this->_leader_position, FOLLOW_RANGE);
        if (arrived == result::FAILED)
            this->_cooldowns[static_cast<size_t>(GOAL::FOLLOW)] = clock::now() + GROUP_RETRY;
        co_return arrived;
    }
    else
    {
        auto now  = clock::now();
        auto pile = std::optional<game_bot::sight::entry>{};
        auto gap  = [&bot](const point<uint16_t>& position) {
            return std::abs(bot.position().x - position.x) + std::abs(bot.position().y - position.y);
        };
        for (auto& [oid, entry] : bot.sight.entries())
        {
            auto avoided    = this->_avoided_targets.find(oid);
            auto unpickable = this->_unpickable.find(entry.look);
            if (entry.name == this->_leader)
                this->_leader_position = entry.position;
            else if (entry.look > ITEM_LOOK_MIN && gap(entry.position) <= ITEM_LOOT_RANGE &&
                     (avoided == this->_avoided_targets.end() || now >= avoided->second) &&
                     (unpickable == this->_unpickable.end() || now >= unpickable->second) &&
                     (pile.has_value() == false || gap(entry.position) < gap(pile->position)) &&
                     this->_knowledge.warp_at(bot.map(), entry.position) == false && this->claims(bot, entry.position))
                pile = entry;
        }

        if (co_await this->conjure(bot, 0))
        {
            co_return result::SUCCEEDED;
        }
        else if (pile.has_value())
        {
            auto outcome = co_await this->walk(bot, pile->position, 0);
            if (outcome == result::SUCCEEDED)
            {
                auto since  = clock::now();
                auto before = bot.items();
                auto cash   = bot.money();
                auto oid    = pile->oid;
                bot.send(*bot.make<game_reqs::loot>(true));
                auto reason = co_await this->expect(
                    bot,
                    since,
                    {"HOLD_FULL", "INVENTORY_FULL", "DROP_WARMTH"},
                    [&bot, &before, cash, oid]() {
                        return bot.sight.entries().contains(oid) == false || bot.money() != cash ||
                               bot.items().size() != before.size() ||
                               std::equal(before.begin(),
                                          before.end(),
                                          bot.items().begin(),
                                          [](const auto& lhs, const auto& rhs) {
                                              return lhs.first == rhs.first && lhs.second.count == rhs.second.count;
                                          }) == false;
                    },
                    ACTION_TIMEOUT);

                auto detail     = Json::Value{};
                detail["look"]  = pile->look;
                detail["group"] = true;
                detail["share"] = true;
                this->report(bot, "loot", reason, detail);

                if (reason == "HOLD_FULL")
                    this->_unpickable.insert_or_assign(pile->look, clock::now() + UNPICKABLE_AVOID);
                else if (reason == "INVENTORY_FULL")
                    this->_sell_urgent = true;
            }

            if (outcome != result::INTERRUPTED && bot.sight.entries().contains(pile->oid))
                this->_avoided_targets.insert_or_assign(pile->oid, clock::now() + TARGET_AVOID);
            co_return outcome;
        }
        else
        {
            co_await bot.thread()->sleep(this->paced(SUPPORT_IDLE));
            co_return result::SUCCEEDED;
        }
    }
}

async::task<brain::result> brain::escape(game_bot& bot)
{
    auto map    = bot.map();
    auto method = std::string{};
    if (bot.state() == STATE::GHOST)
    {
        method = "성황령";
        for (auto direction : {"좌", "우"})
        {
            if (bot.map() == map)
                std::ignore = co_await this->cast(bot, method, direction, 0);
        }
    }
    else if (bot.get_spell_slot_by_name("귀환") != 0xFF && bot.mp() >= RETURN_MP)
    {
        method      = "귀환";
        std::ignore = co_await this->cast(bot, method, {}, 0);
    }
    else if (bot.get_item_slot_by_name("노란비서") != 0xFF)
    {
        method     = "노란비서";
        auto since = clock::now();
        bot.send(*bot.make<game_reqs::item_active>(bot.get_item_slot_by_name(method)));
        std::ignore = co_await this->expect(
            bot,
            since,
            {"GHOST_FORBIDDEN", "WARP_DENIED"},
            [&bot, map]() {
                return bot.map() != map;
            },
            std::chrono::duration_cast<std::chrono::milliseconds>(WARP_TIMEOUT));
    }

    auto escaped     = bot.map() != map;
    auto detail      = Json::Value{};
    detail["from"]   = map;
    detail["method"] = fb::UTF8(method, fb::PLATFORM::WINDOWS);
    this->report(bot, "escape", escaped ? std::string{} : std::string{"NO_MEANS"}, detail);
    if (escaped == false && bot.state() == STATE::GHOST)
        this->_cooldowns[static_cast<size_t>(GOAL::REVIVE)] = clock::now() + ESCAPE_COOLDOWN;
    co_return escaped ? result::SUCCEEDED : result::FAILED;
}

std::optional<std::string> brain::upgrade(const game_bot& bot) const
{
    auto cls    = this->_knowledge.find_class(bot.class_name()).value_or(CLASS::NONE);
    auto items  = table::item;
    auto damage = [](const fb::model::weapon& weapon) {
        return (weapon.damage_small.min + weapon.damage_small.max + weapon.damage_large.min + weapon.damage_large.max) /
               4.0;
    };
    auto wearable = [&](const fb::model::item& item) {
        for (auto& condition : item.condition)
        {
            if (condition.header == DSL::level)
            {
                auto level = fb::model::dsl::level(condition.params);
                if (bot.level() < level.min.value_or(0) || bot.level() > level.max.value_or(0xFF))
                    return false;
            }
            else if (condition.header == DSL::class_t)
            {
                if (fb::model::dsl::class_t(condition.params).value != cls)
                    return false;
            }
            else if (condition.header == DSL::gender)
            {
                if (fb::model::dsl::gender(condition.params).value != bot.gender())
                    return false;
            }
            else
            {
                return false;
            }
        }
        return true;
    };

    auto weapon_now = 0.0;
    auto armor_now  = 0.0;
    auto armor_hue  = std::optional<double>{};
    auto shield_now = 0.0;
    for (auto& [_, item] : items)
    {
        if (auto weapon = dynamic_cast<const fb::model::weapon*>(&item); weapon != nullptr)
        {
            if (bot.weapon_dress() != 0 && weapon->dress == bot.weapon_dress())
                weapon_now = std::max(weapon_now, damage(*weapon));
        }
        else if (auto armor = dynamic_cast<const fb::model::armor*>(&item); armor != nullptr)
        {
            if (armor->dress == bot.armor_dress())
                armor_now = std::max(armor_now, static_cast<double>(armor->defensive_physical));
            if (armor->dress == bot.armor_dress() && armor->color == bot.armor_color())
                armor_hue = std::max(armor_hue.value_or(0.0), static_cast<double>(armor->defensive_physical));
        }
        else if (auto shield = dynamic_cast<const fb::model::shield*>(&item); shield != nullptr)
        {
            if (bot.shield_dress() != 0 && shield->dress == bot.shield_dress())
                shield_now = std::max(shield_now, static_cast<double>(shield->defensive_physical));
        }
    }

    if (armor_hue.has_value())
        armor_now = armor_hue.value();

    auto spare = uint64_t{0};
    for (auto lesson :
         this->_spellbook.pending(cls,
                                  bot,
                                  static_cast<uint8_t>(std::min(255, bot.level() + this->_spellbook.lookahead))))
    {
        spare += lesson->money;
    }

    auto best = std::optional<std::string>{};
    auto gain = 0.0;
    for (auto& [_, shop] : this->_knowledge.shops())
    {
        for (auto& [name, price] : shop.sells)
        {
            auto equipment = dynamic_cast<const fb::model::equipment*>(items->name2item(name));
            if (equipment == nullptr || this->_tried_gear.contains(name) || wearable(*equipment) == false)
                continue;

            if (price + spare > bot.money() && bot.get_item_count_by_name(name) == 0)
                continue;

            auto value = 0.0;
            if (auto weapon = dynamic_cast<const fb::model::weapon*>(equipment); weapon != nullptr)
                value = damage(*weapon) >= weapon_now * WEAPON_UPGRADE
                            ? damage(*weapon) / std::max(weapon_now, 1.0) - 1.0
                            : 0.0;
            else if (dynamic_cast<const fb::model::armor*>(equipment) != nullptr)
                value = equipment->defensive_physical >= armor_now + ARMOR_UPGRADE
                            ? (equipment->defensive_physical - armor_now) / std::max(armor_now, ARMOR_UPGRADE)
                            : 0.0;
            else if (dynamic_cast<const fb::model::shield*>(equipment) != nullptr)
                value = equipment->defensive_physical >= shield_now + ARMOR_UPGRADE
                            ? (equipment->defensive_physical - shield_now) / std::max(shield_now, ARMOR_UPGRADE)
                            : 0.0;

            if (value > gain)
            {
                gain = value;
                best = name;
            }
        }
    }
    return best;
}

async::task<brain::result> brain::equip(game_bot& bot)
{
    auto name = this->upgrade(bot);
    if (name.has_value() == false)
        co_return result::FAILED;

    if (bot.get_item_count_by_name(name.value()) == 0)
    {
        auto bought = co_await this->buy(bot, name.value(), 1);
        if (bought == result::INTERRUPTED)
            co_return bought;
    }

    auto slot = bot.get_item_slot_by_name(name.value());
    auto worn = false;
    if (slot != 0xFF)
        worn = co_await bot.equip(slot, std::chrono::duration_cast<std::chrono::milliseconds>(DIALOG_TIMEOUT));

    auto detail    = Json::Value{};
    detail["item"] = fb::UTF8(name.value(), fb::PLATFORM::WINDOWS);
    this->report(bot, "equip", worn ? std::string{} : std::string{"NOT_WORN"}, detail);
    this->_tried_gear.insert(name.value());
    this->_cooldowns[static_cast<size_t>(GOAL::EQUIP)] = clock::now() + (worn ? EQUIP_SETTLE : SHOP_AVOID);
    co_return worn ? result::SUCCEEDED : result::FAILED;
}

async::task<brain::result> brain::use(game_bot& bot, const std::string& item)
{
    auto slot = bot.get_item_slot_by_name(item);
    if (slot == 0xFF)
        co_return result::FAILED;

    auto dialog     = std::optional<integration::dialog_bot>{};
    auto dialog_ext = std::optional<integration::dialog_ext_bot>{};
    auto seq        = bot.last_dialog(dialog, dialog_ext);
    bot.send(*bot.make<game_reqs::item_active>(slot));

    std::ignore = co_await this->answer(bot, seq);
    if (this->alive(bot) == false)
        co_return result::INTERRUPTED;

    co_return result::SUCCEEDED;
}

async::task<uint32_t> brain::answer(game_bot& bot, uint32_t seq, const std::vector<std::string>& picks)
{
    auto dialog     = std::optional<integration::dialog_bot>{};
    auto dialog_ext = std::optional<integration::dialog_ext_bot>{};
    auto answered   = uint32_t{0};
    auto waiting    = true;
    while (waiting)
    {
        auto deadline = clock::now() + DIALOG_TIMEOUT;
        while (this->alive(bot) && bot.last_dialog(dialog, dialog_ext) == seq && clock::now() < deadline)
        {
            co_await bot.thread()->sleep(100ms);
        }

        auto current = bot.last_dialog(dialog, dialog_ext);
        if (this->alive(bot) == false || current == seq || answered >= DIALOG_LIMIT)
        {
            waiting = false;
        }
        else
        {
            seq = current;
            answered++;
            auto listed = dialog_ext.has_value() && picks.empty() == false &&
                          (dialog_ext->type == integration::dialog_ext_type::list ||
                           dialog_ext->type == integration::dialog_ext_type::list_no_msg);
            if (listed)
            {
                auto& lists      = dialog_ext->list_lists;
                auto  found      = std::find_first_of(lists.begin(), lists.end(), picks.begin(), picks.end());
                auto  entry      = Json::Value{};
                entry["event"]   = "dialog";
                entry["message"] = fb::UTF8(dialog_ext->message, fb::PLATFORM::WINDOWS);
                entry["menus"]   = Json::Value{Json::arrayValue};
                for (auto& item : lists)
                {
                    entry["menus"].append(fb::UTF8(item, fb::PLATFORM::WINDOWS));
                }
                entry["index"] = found != lists.end() ? static_cast<int>(std::distance(lists.begin(), found)) : -1;
                this->_trace.write(bot.name(), entry);

                if (found == lists.end())
                    waiting = false;
                else
                    bot.send(*bot.make<game_reqs::dialog_list>(
                        dialog_ext->type == integration::dialog_ext_type::list
                            ? fb::game::dialog::list_type::LIST
                            : fb::game::dialog::list_type::LIST_NO_MSG,
                        uint8_t{0},
                        std::string{},
                        static_cast<uint16_t>(std::distance(lists.begin(), found) + 1),
                        DIALOG_RESULT::NEXT));
            }
            else if (dialog_ext.has_value())
                bot.send(*bot.make<game_reqs::dialog_list>(fb::game::dialog::list_type::TEXT,
                                                           static_cast<uint8_t>(DIALOG_RESULT::NEXT),
                                                           std::string{},
                                                           uint16_t{0},
                                                           DIALOG_RESULT::NEXT));
            else
            {
                auto index  = size_t{0};
                auto missed = false;
                if (dialog.has_value())
                {
                    auto& menus = dialog->menu_menus;
                    auto  found = std::find_first_of(menus.begin(), menus.end(), picks.begin(), picks.end());
                    if (found != menus.end())
                        index = static_cast<size_t>(std::distance(menus.begin(), found));
                    else
                        missed = picks.empty() == false && menus.empty() == false;
                }

                if (picks.empty() == false && dialog.has_value())
                {
                    auto entry       = Json::Value{};
                    entry["event"]   = "dialog";
                    entry["type"]    = static_cast<int>(dialog->type);
                    entry["message"] = fb::UTF8(dialog->message, fb::PLATFORM::WINDOWS);
                    entry["menus"]   = Json::Value{Json::arrayValue};
                    for (auto& menu : dialog->menu_menus)
                    {
                        entry["menus"].append(fb::UTF8(menu, fb::PLATFORM::WINDOWS));
                    }
                    entry["index"]  = static_cast<uint32_t>(index);
                    entry["missed"] = missed;
                    this->_trace.write(bot.name(), entry);
                }

                if (missed)
                    waiting = false;
                else if (dialog.has_value() && dialog->type == integration::dialog_type::pursuit &&
                         dialog->menu_menus.empty() == false)
                    bot.send(*bot.make<game_reqs::dialog>(fb::game::dialog::type::PURSUIT,
                                                          uint8_t{0},
                                                          std::string{},
                                                          uint16_t{0},
                                                          dialog->pursuit,
                                                          dialog->menu_menus.at(index)));
                else
                    bot.send(*bot.make<game_reqs::dialog>(fb::game::dialog::type::MENU,
                                                          uint8_t{0},
                                                          std::string{},
                                                          static_cast<uint16_t>(index + 1),
                                                          uint16_t{0},
                                                          std::string{}));
            }
        }
    }

    if (answered > 0 && this->alive(bot))
        bot.send(*bot.make<game_reqs::click>(DIALOG_CLOSE_OID,
                                             game_reqs::click<fb::protocol::CLIENT_VERSION::v550>::FLAG_OBJECT));
    co_return answered;
}
