#ifndef __BOT_LOAD_BRAIN_H__
#define __BOT_LOAD_BRAIN_H__

#include <fb/bot/cluster/journal.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/load/knowledge.h>
#include <fb/bot/load/messages.h>
#include <fb/bot/load/trace.h>

#include <json/json.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fb::bot::load {

enum class GOAL : uint8_t
{
    REVIVE   = 0,
    REST     = 1,
    TUTORIAL = 2,
    JOB      = 3,
    SELL     = 4,
    SUPPLY   = 5,
    HUNT     = 6,
    WANDER   = 7,
    LEARN    = 8,
    EQUIP    = 9,
    GROUP    = 10,
    FOLLOW   = 11,
    RECLAIM  = 12,
    TRADE    = 13,
    QUEST    = 14,
};

constexpr size_t GOAL_COUNT = 15;

// clang-format off
constexpr auto GOAL_NAMES = std::array<const char*, GOAL_COUNT>{"REVIVE", "REST", "TUTORIAL", "JOB", "SELL", "SUPPLY", "HUNT", "WANDER", "LEARN", "EQUIP", "GROUP", "FOLLOW", "RECLAIM", "TRADE", "QUEST"};
// clang-format on
constexpr size_t CLASS_COUNT = 5;

struct persona
{
    std::string               name;
    double                    ratio            = 1.0;
    double                    hunt             = 1.0;
    double                    rest             = 1.0;
    double                    tutorial         = 1.5;
    double                    job              = 1.4;
    double                    sell             = 1.3;
    double                    supply           = 1.2;
    double                    learn            = 2.0;
    double                    equip            = 1.5;
    double                    group            = 1.6;
    double                    follow           = 2.2;
    double                    reclaim          = 1.8;
    double                    trade            = 1.4;
    double                    quest            = 1.8;
    double                    wander           = 0.05;
    double                    jitter           = 0.1;
    double                    rest_below       = 0.35;
    double                    rest_until       = 0.9;
    double                    max_damage_ratio = 0.2;
    double                    max_threat_ratio = 0.04;
    double                    max_death_rate   = 0.05;
    double                    max_loss_ratio   = 0.7;
    double                    explore_rate     = 0.1;
    double                    abort_hp_ratio   = 0.3;
    double                    temperature      = 0.3;
    double                    money_weight     = 1.0;
    double                    distance         = 1.0;
    bool                      learning         = false;
    std::chrono::milliseconds step_delay       = std::chrono::milliseconds(300);
    std::chrono::milliseconds attack_delay     = std::chrono::milliseconds(600);
    std::chrono::milliseconds attack_wait      = std::chrono::milliseconds(600);
    double                    speed            = 1.0;

    persona() = default;
    persona(const Json::Value& json);

    void    accelerate(double factor);
    persona vary(const Json::Value& spread, std::mt19937& random) const;
};

struct tutorial
{
    enum class TASK : uint8_t
    {
        NONE   = 0,
        USE    = 1,
        TRAVEL = 2,
        HUNT   = 3,
    };

    struct step
    {
        std::string item;
        TASK        task = TASK::NONE;
        std::string target;
        uint32_t    count = 1;
    };

    uint16_t          giver_map = 0xFFFF;
    uint32_t          giver_npc = 0;
    std::vector<step> steps;

    tutorial() = default;
    tutorial(const Json::Value& json);
};

struct job
{
    uint8_t                               level       = 5;
    uint8_t                               group_level = 15;
    uint8_t                               group_gap   = 8;
    std::chrono::seconds                  group_apart = std::chrono::seconds(300);
    std::string                           item;
    uint32_t                              count = 0;
    std::string                           ground;
    std::unordered_map<CLASS, uint32_t>   npcs;
    std::vector<std::pair<CLASS, double>> demand;

    job() = default;
    job(const Json::Value& json);
};

struct supplies
{
    struct remedy
    {
        std::string item;
        uint32_t    hp       = 0;
        uint32_t    mp       = 0;
        double      hp_ratio = 0.0;
        double      mp_ratio = 0.0;

        uint32_t healing(uint32_t max_hp) const;
        uint32_t refill(uint32_t max_mp) const;
    };

    std::vector<remedy>             remedies;
    std::unordered_set<std::string> travels;
    uint8_t                         sell_slots = 40;

    supplies() = default;
    supplies(const Json::Value& catalog, const Json::Value& config);

    const remedy* find(const std::string& item) const;
};

struct spellbook
{
    enum class ROLE : uint8_t
    {
        HEAL    = 0,
        ATTACK  = 1,
        BUFF    = 2,
        OTHER   = 3,
        AREA    = 4,
        STRIKE  = 5,
        AMBUSH  = 6,
        ENCHANT = 7,
        DEBUFF  = 8,
        CURE    = 9,
        CHARGE  = 10,
        TRAVEL  = 11,
        GOTO    = 12,
        ESCAPE  = 13,
        SHOUT   = 14,
    };

    enum class TRIGGER : uint8_t
    {
        ALWAYS    = 0,
        EMERGENCY = 1,
        MANA      = 2,
    };

    struct lesson
    {
        std::vector<std::string>                      names;
        uint8_t                                       level = 0;
        uint32_t                                      money = 0;
        std::vector<std::pair<std::string, uint32_t>> materials;
    };

    struct guild
    {
        uint32_t            npc = 0;
        std::vector<lesson> lessons;
    };

    struct effect
    {
        uint32_t    id        = 0;
        SPELL_TYPE  type      = SPELL_TYPE::NORMAL;
        ROLE        role      = ROLE::OTHER;
        uint32_t    mp        = 0;
        uint32_t    hp        = 0;
        uint32_t    damage    = 0;
        uint32_t    buff_time = 0;
        bool        self      = false;
        bool        exclusive = false;
        TRIGGER     trigger   = TRIGGER::ALWAYS;
        std::string cure;
        double      hp_cost   = 0.0;
        uint32_t    damage_hp = 0;
    };

    std::unordered_map<CLASS, guild>        guilds;
    std::unordered_map<std::string, effect> effects;
    uint8_t                                 lookahead = 5;

    spellbook() = default;
    spellbook(const Json::Value& catalog, const Json::Value& config);

    std::vector<const lesson*> pending(CLASS cls, const game_bot& bot, uint8_t level) const;
    const effect*              find(const std::string& name) const;
};

struct questbook
{
    struct quest
    {
        uint32_t                                  id = 0;
        std::vector<uint32_t>                     npcs;
        uint8_t                                   level = 0;
        std::unordered_map<std::string, uint32_t> consumes;
        uint32_t                                  money = 0;
        std::unordered_map<std::string, uint32_t> rewards;
        uint32_t                                  reward_money = 0;
        uint32_t                                  reward_exp   = 0;
        std::vector<std::string>                  picks;
    };

    std::vector<quest>                                               quests;
    std::unordered_map<std::string, std::unordered_set<std::string>> hunts;

    questbook() = default;
    questbook(const Json::Value& catalog, const Json::Value& config);
};

class census
{
private:
    mutable std::mutex                     _mutex;
    std::array<uint32_t, CLASS_COUNT>      _population{};
    std::unordered_map<std::string, CLASS> _reservations;
    cluster::journal                       _journal;

public:
    void                 update(const std::array<uint32_t, CLASS_COUNT>& population);
    std::optional<CLASS> reserve(const std::string&                           name,
                                 const std::vector<std::pair<CLASS, double>>& demand,
                                 std::mt19937&                                random);
    void                 release(const std::string& name);
    cluster::journal&    journal();
    void                 apply(const Json::Value& event);
};

class hazards
{
private:
    mutable std::mutex                                         _mutex;
    std::unordered_map<uint16_t, std::unordered_set<uint32_t>> _tiles;
    cluster::journal                                           _journal;

public:
    bool                         add(uint16_t map, const point<uint16_t>& position);
    std::unordered_set<uint32_t> tiles(uint16_t map) const;
    cluster::journal&            journal();
    void                         apply(const Json::Value& event);
};

class crowd
{
private:
    using clock = std::chrono::steady_clock;

    struct presence
    {
        uint16_t          map = 0;
        clock::time_point seen;
    };

    struct passage
    {
        uint16_t          map = 0;
        point<uint16_t>   tile;
        int64_t           asked = 0;
        clock::time_point seen;
    };

private:
    mutable std::mutex                             _mutex;
    std::unordered_map<std::string, presence>      _presence;
    std::unordered_map<std::string, presence>      _heading;
    std::unordered_map<std::string, presence>      _shared;
    mutable std::unordered_map<uint16_t, uint32_t> _tally;
    mutable clock::time_point                      _tallied;
    std::unordered_map<std::string, passage>       _passages;
    cluster::journal                               _journal;

public:
    void                         see(const std::string& self, uint16_t map, const std::vector<std::string>& others);
    void                         head(const std::string& self, uint16_t map);
    uint32_t                     count(uint16_t map, const std::string& except) const;
    void                         ask(const std::string& self, uint16_t map, const point<uint16_t>& tile);
    std::vector<point<uint16_t>> passages(uint16_t map, const std::string& self) const;
    knowledge::queue_map         queues() const;
    cluster::journal&            journal();
    void                         apply(const Json::Value& event);
};

class learner
{
public:
    static constexpr size_t FEATURES = 8;
    using features                   = std::array<double, FEATURES>;

private:
    mutable std::mutex _mutex;
    features           _theta{};
    uint64_t           _samples = 0;
    double             _alpha   = 0.02;
    double             _error   = 0.0;
    std::string        _path;
    cluster::journal   _journal;

public:
    void              configure(const Json::Value& json);
    double            predict(const features& input) const;
    void              add(const features& input, double target);
    void              save() const;
    Json::Value       state() const;
    cluster::journal& journal();
    void              apply(const Json::Value& event);

private:
    void update(const features& input, double target);
};

class roster
{
private:
    mutable std::mutex                       _mutex;
    std::unordered_map<std::string, uint8_t> _levels;
    std::unordered_set<std::string>          _grouped;
    cluster::journal                         _journal;

public:
    void                   update(const std::string& name, uint8_t level);
    void                   remove(const std::string& name);
    void                   group(const std::string& name, bool joined);
    std::optional<uint8_t> level(const std::string& name) const;
    bool                   grouped(const std::string& name) const;
    cluster::journal&      journal();
    void                   apply(const Json::Value& event);
};

class market
{
public:
    struct want
    {
        std::string                           name;
        std::string                           item;
        uint32_t                              count = 0;
        uint16_t                              map   = 0;
        point<uint16_t>                       position;
        std::string                           giver;
        std::chrono::steady_clock::time_point claimed{};
    };

private:
    mutable std::mutex _mutex;
    std::vector<want>  _wants;
    cluster::journal   _journal;

public:
    bool                post(const std::string&     name,
                             const std::string&     item,
                             uint32_t               count,
                             uint16_t               map,
                             const point<uint16_t>& position);
    void                withdraw(const std::string& name, const std::string& item);
    void                release(const std::string& giver);
    bool                wanted(const std::string& item, const std::string& except) const;
    std::optional<want> find(const std::string& name, const std::string& item) const;
    std::optional<want> peek(const std::string& giver, const std::function<bool(const want&)>& accept) const;
    std::optional<want> claim(const std::string& giver, const std::function<bool(const want&)>& accept);
    cluster::journal&   journal();
    void                apply(const Json::Value& event);
};

class experience
{
public:
    enum class OUTCOME : uint8_t
    {
        KILLED  = 0,
        ABORTED = 1,
        DIED    = 2,
    };

    struct record
    {
        uint32_t fights  = 0;
        uint32_t deaths  = 0;
        uint32_t aborts  = 0;
        double   seconds = 0.0;
        double   damage  = 0.0;
        double   loss    = 0.0;
        double   exp     = 0.0;
    };

    struct fight
    {
        std::string mob;
        OUTCOME     outcome;
        double      seconds;
        double      damage;
        double      loss;
        double      exp;
    };

private:
    mutable std::mutex                                  _mutex;
    uint8_t                                             _level_band = 10;
    std::unordered_map<CLASS, std::vector<std::string>> _skills;
    std::unordered_map<std::string, record>             _records;
    uint64_t                                            _fights          = 0;
    double                                              _multiplier      = 1.0;
    double                                              _drop_multiplier = 1.0;
    bool                                                _limited         = true;
    cluster::journal                                    _journal;

public:
    void                     configure(const Json::Value& json);
    double                   multiplier() const;
    double                   drop_multiplier() const;
    bool                     limited() const;
    std::vector<std::string> keys(const game_bot& bot, CLASS cls) const;
    void                     add(const std::vector<std::string>& keys, const fight& value);
    std::optional<record>    find(const std::vector<std::string>& keys, const std::string& mob) const;
    uint64_t                 fights() const;
    cluster::journal&        journal();
    void                     apply(const Json::Value& event);

private:
    void merge(const std::string& key, const fight& value);
};

struct metrics
{
    std::atomic<uint64_t>                         kills  = 0;
    std::atomic<uint64_t>                         deaths = 0;
    std::atomic<uint64_t>                         exp    = 0;
    std::atomic<uint64_t>                         stuck  = 0;
    std::array<std::atomic<uint64_t>, GOAL_COUNT> failures{};
};

class brain
{
public:
    enum class result : uint8_t
    {
        SUCCEEDED   = 0,
        FAILED      = 1,
        INTERRUPTED = 2,
    };

private:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;

    struct visit
    {
        uint16_t          map = 0;
        learner::features input{};
        double            base = 0.0;
        time_point        since{};
        double            travel = 0.0;
        double            hunted = 0.0;
        uint32_t          gained = 0;
        bool              failed = false;
    };

private:
    // clang-format off
    const knowledge&                                           _knowledge;
    const persona                                              _persona;
    const tutorial&                                            _tutorial;
    const job&                                                 _job;
    const supplies&                                            _supplies;
    const spellbook&                                           _spellbook;
    const questbook&                                           _questbook;
    census&                                                    _census;
    hazards&                                                   _hazards;
    market&                                                    _market;
    roster&                                                    _roster;
    crowd&                                                     _crowd;
    learner&                                                   _learner;
    experience&                                                _experience;
    const messages&                                            _messages;
    trace&                                                     _trace;
    metrics&                                                   _metrics;
    std::mt19937                                               _random;
    std::atomic<uint32_t>                                      _owner   = 0;
    std::atomic<bool>                                          _running = false;
    std::atomic<int>                                           _goal    = -1;
    std::atomic<int>                                           _class   = -1;
    std::atomic<int>                                           _level   = 0;
    bool                                                       _interrupted = false;
    std::array<time_point, GOAL_COUNT>                         _cooldowns{};
    std::array<uint32_t, GOAL_COUNT>                           _failed_streak{};
    std::unordered_map<uint16_t, time_point>                   _avoided_grounds;
    std::unordered_map<uint32_t, time_point>                   _avoided_targets;
    std::unordered_map<std::string, time_point>                _avoided_steps;
    std::unordered_map<std::string, uint32_t>                  _step_attempts;
    std::unordered_map<uint16_t, std::unordered_map<uint32_t, time_point>> _learned_blocks;
    std::optional<knowledge::hunting_ground>                   _ground;
    uint32_t                                                   _last_exp  = 0;
    bool                                                       _was_ghost = false;
    uint16_t                                                   _progress_map = 0xFFFF;
    point<uint16_t>                                            _progress_position;
    uint32_t                                                   _progress_exp = 0;
    time_point                                                 _progress_time;
    time_point                                                 _info_time;
    time_point                                                 _probe_sent{};
    bool                                                       _probe_pending = false;
    uint32_t                                                   _lag_samples   = 0;
    std::unordered_map<std::string, time_point>                _heard;
    std::unordered_map<uint32_t, std::pair<time_point, uint32_t>> _hits;
    uint32_t                                                   _fighting     = 0;
    std::unordered_map<uint32_t, std::string>                  _names;
    std::unordered_map<uint16_t, std::pair<double, double>>    _yields;
    uint32_t                                                   _clicked      = 0;
    time_point                                                 _clicked_at{};
    uint64_t                                                   _fight_damage = 0;
    std::unordered_map<uint16_t, time_point>                   _unpickable;
    std::unordered_map<uint32_t, std::unordered_set<std::string>> _rejected_trades;
    std::unordered_set<std::string>                            _tried_gear;
    uint16_t                                                   _enchanted = 0;
    uint32_t                                                   _misfires  = 0;
    std::unordered_set<uint32_t>                               _quests_done;
    std::unordered_map<uint32_t, time_point>                   _quest_shun;
    std::unordered_map<uint32_t, uint32_t>                     _quest_stops;
    uint32_t                                                   _quest_active = 0;
    std::unordered_map<uint16_t, time_point>                   _hurt_maps;
    uint16_t                                                   _sampled_map = 0xFFFF;
    uint32_t                                                   _sampled_hp  = 0;
    time_point                                                 _stated{};
    time_point                                                 _crowded{};
    time_point                                                 _tutorial_grace{};
    mutable time_point                                         _quest_checked{};
    mutable const questbook::quest*                            _quest_choice = nullptr;
    mutable std::string                                        _quest_why;
    uint16_t                                                   _remains_map = 0xFFFF;
    point<uint16_t>                                            _remains_position;
    time_point                                                 _remains_until{};
    bool                                                       _sell_urgent = false;
    bool                                                       _clearing    = false;
    double                                                     _dps         = -1.0;
    double                                                     _armor       = 1.0;
    time_point                                                 _escaped_until;
    std::unordered_map<std::string, time_point>                _spell_waits;
    std::unordered_map<uint32_t, uint32_t>                     _vitals;
    bool                                                       _grouped     = false;
    bool                                                       _group_ready = false;
    std::string                                                _leader;
    uint16_t                                                   _leader_map  = 0xFFFF;
    point<uint16_t>                                            _leader_position;
    time_point                                                 _leader_seen;
    time_point                                                 _announced;
    time_point                                                 _advertised;
    std::string                                                _partner;
    time_point                                                 _partner_seen;
    std::unordered_set<std::string>                            _wanted;
    std::unordered_map<std::string, time_point>                _group_tries;
    std::unordered_map<uint32_t, time_point>                   _piles;
    uint32_t                                                   _opponent_damage = 0;
    bool                                                       _fight_spells    = false;
    time_point                                                 _struck{};
    double                                                     _mp_gain         = 0.0;
    double                                                     _mp_time         = 0.0;
    uint32_t                                                   _mp_seen         = 0;
    time_point                                                 _mp_since{};
    time_point                                                 _drank{};
    double                                                     _travel_seconds  = 0.0;
    time_point                                                 _born{};
    double                                                     _stay            = 0.0;
    double                                                     _loss_rate       = 0.0;
    std::unordered_map<std::string, std::pair<double, double>> _observed;
    std::optional<visit>                                       _visit;
    std::optional<std::pair<uint16_t, time_point>>             _cornered;
    std::unordered_map<uint64_t, time_point>                   _jammed;
    std::deque<std::pair<uint16_t, time_point>>                _blocks;
    std::unordered_map<std::string, time_point>                _unlearnable;
    mutable std::pair<double, double>                          _powered{-1.0, 0.0};
    mutable time_point                                         _powered_at{};
    mutable std::pair<time_point, std::optional<std::pair<std::string, uint32_t>>> _shortage;
    mutable std::pair<time_point, std::optional<std::string>>                       _upgrade;
    mutable std::unordered_map<uint8_t, std::pair<time_point, std::vector<const spellbook::lesson*>>> _lessons;
    // clang-format on

public:
    brain(const knowledge& knowledge,
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
          crowd&           crowd,
          learner&         learner,
          experience&      experience,
          const messages&  messages,
          trace&           trace,
          metrics&         metrics,
          uint32_t         seed);

public:
    async::task<void>    run(std::shared_ptr<game_bot> bot);
    void                 check(game_bot& bot);
    std::optional<GOAL>  goal() const;
    std::optional<CLASS> cls() const;
    uint8_t              level() const;

private:
    // clang-format off
    bool                                     alive(const game_bot& bot) const;
    void                                     perceive(game_bot& bot);
    bool                                     heard(const std::string& kind, time_point since) const;
    async::task<std::string>                 expect(game_bot& bot, time_point since, const std::vector<std::string>& failures, const std::function<bool()>& done, std::chrono::milliseconds timeout);
    void                                     report(game_bot& bot, const char* action, const std::string& reason, Json::Value detail);
    double                                   score(GOAL goal, const game_bot& bot) const;
    bool                                     dangerous(const game_bot& bot, const knowledge::prey& prey, const std::vector<std::string>& keys) const;
    double                                   power(const game_bot& bot, double seconds, const spellbook::effect* extra = nullptr) const;
    double                                   regen() const;
    void                                     blocked(uint16_t map);
    uint32_t                                 jam(uint16_t map) const;
    knowledge::detour_map                    detours() const;
    knowledge::queue_map                     queues() const;
    double                                   worth(const game_bot& bot, const spellbook::lesson& lesson) const;
    std::vector<const spellbook::lesson*>    lessons(const game_bot& bot, uint8_t level) const;
    uint32_t                                 healing(const game_bot& bot, const supplies::remedy& remedy) const;
    uint32_t                                 refill(const game_bot& bot, const supplies::remedy& remedy) const;
    uint32_t                                 need(const game_bot& bot, const std::string& item) const;
    std::chrono::milliseconds                paced(std::chrono::milliseconds value) const;
    bool                                     harmful(const game_bot& bot, const knowledge::prey& prey, const std::vector<std::string>& keys) const;
    const knowledge::prey*                   find_prey(uint16_t map, uint16_t look, uint32_t oid) const;
    const knowledge::prey*                   identify(game_bot& bot, const knowledge::hunting_ground& ground, const game_bot::sight::entry& entry);
    std::pair<uint32_t, uint32_t>            safe_preys(const game_bot& bot, const knowledge::hunting_ground& ground, const std::vector<std::string>& keys) const;
    std::optional<knowledge::hunting_ground> select_ground(const game_bot& bot, const std::unordered_set<std::string>& preys = {});
    async::task<void>                        retreat(game_bot& bot);
    std::optional<knowledge::hunting_ground> find_ground(const game_bot& bot, const std::string& suffix) const;
    uint32_t                                 congestion(const game_bot& bot, const std::unordered_map<uint16_t, knowledge::visit>& visits, uint16_t map) const;
    const tutorial::step*                    find_step(const game_bot& bot) const;
    std::optional<knowledge::npc_spot>       nearest(const game_bot& bot, const std::vector<knowledge::npc_spot>& spots) const;
    async::task<result>                      revive(game_bot& bot);
    async::task<result>                      rest(game_bot& bot);
    async::task<result>                      appreciate(game_bot& bot);
    async::task<result>                      learn(game_bot& bot);
    async::task<result>                      join(game_bot& bot);
    async::task<result>                      study(game_bot& bot);
    std::unordered_map<std::string, uint32_t> reserved(const game_bot& bot) const;
    static std::string                       stack_name(const std::string& name);
    std::unordered_map<std::string, uint32_t> owned(const game_bot& bot) const;
    std::vector<std::pair<std::string, uint32_t>> sellable(const game_bot& bot) const;
    bool                                     overstocked(const game_bot& bot) const;
    bool                                     underfunded(const game_bot& bot) const;
    async::task<result>                      sell(game_bot& bot);
    async::task<result>                      supply(game_bot& bot);
    async::task<result>                      buy(game_bot& bot, const std::string& item, uint32_t total, uint64_t keep = 0);
    std::optional<std::pair<std::string, uint32_t>> shortage(const game_bot& bot) const;
    uint64_t                                 savings(const game_bot& bot) const;
    std::optional<uint64_t>                  tuition(const game_bot& bot, const spellbook::lesson& lesson) const;
    const spellbook::lesson*                 affordable(const game_bot& bot) const;
    async::task<bool>                        mend(game_bot& bot);
    uint32_t                                 held(const game_bot& bot, const std::string& item, const std::unordered_map<std::string, uint32_t>& reserved) const;
    async::task<result>                      escape(game_bot& bot);
    std::optional<std::string>               upgrade(const game_bot& bot) const;
    async::task<result>                      equip(game_bot& bot);
    async::task<bool>                        cast(game_bot& bot, const std::string& name, const std::string& message, uint32_t oid);
    async::task<bool>                        conjure(game_bot& bot, uint32_t target);
    async::task<result>                      group(game_bot& bot);
    async::task<result>                      follow(game_bot& bot);
    void                                     announce(game_bot& bot);
    void                                     advertise(game_bot& bot);
    void                                     regroup(game_bot& bot);
    bool                                     claims(const game_bot& bot, const point<uint16_t>& position);
    bool                                     surplus(const game_bot& bot, const market::want& want, uint32_t need) const;
    async::task<result>                      trade(game_bot& bot);
    const questbook::quest*                  choose_quest(const game_bot& bot) const;
    async::task<result>                      quest(game_bot& bot);
    bool                                     hostile(const game_bot& bot) const;
    bool                                     leader_hunting() const;
    async::task<bool>                        eat(game_bot& bot);
    async::task<bool>                        drink(game_bot& bot);
    async::task<bool>                        enchant(game_bot& bot);
    async::task<result>                      reclaim(game_bot& bot);
    bool                                     caster(const game_bot& bot) const;
    async::task<result>                      hunt(game_bot& bot);
    async::task<result>                      farm(game_bot& bot, const knowledge::hunting_ground& ground, std::chrono::seconds slice);
    async::task<result>                      wander(game_bot& bot);
    async::task<void>                        yield(game_bot& bot);
    async::task<result>                      travel(game_bot& bot, uint16_t map, const point<uint16_t>& target, uint16_t range);
    async::task<result>                      walk(game_bot& bot, const point<uint16_t>& target, uint16_t range);
    async::task<result>                      fight(game_bot& bot, uint32_t oid);
    std::vector<std::string>                 experience_keys(const game_bot& bot) const;
    async::task<result>                      talk(game_bot& bot, uint32_t oid, const std::vector<std::string>& picks = {});
    async::task<result>                      use(game_bot& bot, const std::string& item);
    async::task<uint32_t>                    answer(game_bot& bot, uint32_t seq, const std::vector<std::string>& picks = {});
    // clang-format on
};

} // namespace fb::bot::load

#endif
