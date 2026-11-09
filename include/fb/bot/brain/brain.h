#ifndef __FB_BOT_BRAIN_BRAIN_H__
#define __FB_BOT_BRAIN_BRAIN_H__

#include <fb/bot/brain/atlas.h>
#include <fb/bot/game_bot.h>
#include <fb/lua.h>
#include <fb/model/model.h>

#include <async/task.h>
#include <async/task_completion_source.h>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <thread>
#include <type_traits>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fb::bot {

class brain : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

public:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;

    enum class STATE : uint8_t
    {
        DEAD     = 0,
        PREPARE  = 1,
        TRAVEL   = 2,
        HUNT     = 3,
        RETURN   = 4,
        PROGRESS = 5,
    };

    enum class ACTION : uint8_t
    {
        TASK     = 0,
        CONTINUE = 1,
        HOLD     = 2,
        MOVE     = 3,
        ATTACK   = 4,
        TALK     = 5,
        READ     = 6,
        USE      = 7,
        PICK     = 8,
        EQUIP    = 9,
        WAIT     = 10,
        FORGET   = 11,
        CAST     = 12,
        SAY      = 13,
        INVITE   = 14,
        LEAVE    = 15,
    };

    enum class MANEUVER : uint8_t
    {
        SWING = 0,
        CAST  = 1,
        USE   = 2,
    };

    enum class ROLE : uint8_t
    {
        SOLO   = 0,
        LEADER = 1,
        MEMBER = 2,
    };

    struct snapshot
    {
        uint8_t                      level     = 0;
        fb::model::enum_value::CLASS cls       = fb::model::enum_value::CLASS::NONE;
        uint8_t                      promotion = 0;
        bool                         ghost     = false;
        uint16_t                     map       = 0;
        uint8_t                      nation    = 0;
    };

    struct decision
    {
        STATE                     state = STATE::PREPARE;
        std::string               task;
        uint8_t                   step = 0;
        std::string               reason;
        ACTION                    kind = ACTION::TASK;
        fb::lua::value            action;
        std::chrono::milliseconds revisit{0};
    };

    struct transition
    {
        decision   to;
        time_point at;
    };

    struct limit
    {
        std::string task;
        uint8_t     step = 0;
    };

    struct profile
    {
        std::unordered_map<std::string, double>         persona;
        uint32_t                                        seed = 0;
        std::optional<limit>                            halt;
        std::unordered_map<std::string, fb::lua::value> memory;
    };

    struct leash
    {
        std::unordered_set<std::string> names;
        uint16_t                        x = 0;
        uint16_t                        y = 0;
    };

    struct destination
    {
        std::unordered_set<uint16_t>   maps;
        std::optional<atlas::position> at;
        uint16_t                       range = 0;
        uint16_t                       steps = 0;
        std::chrono::milliseconds      pace  = std::chrono::milliseconds{0};
        std::optional<struct leash>    leash;
    };

    struct jump
    {
        atlas::teleport                                  teleport;
        std::string                                      spell;
        std::string                                      item;
        std::unordered_map<atlas::CARDINAL, std::string> inputs;
    };

    struct detour
    {
        std::unordered_set<uint64_t> closed;
        std::unordered_set<uint64_t> avoid;
        std::unordered_set<uint32_t> perilous;
        std::unordered_set<uint16_t> shunned;
        std::vector<jump>            jumps;
        std::optional<size_t>        leap;
        atlas::CARDINAL              side = atlas::CARDINAL::EAST;
    };

    struct listing
    {
        uint16_t       map = 0;
        fb::lua::value profile;
    };

    struct maneuver
    {
        MANEUVER    kind = MANEUVER::SWING;
        uint8_t     slot = 0;
        bool        self = false;
        int32_t     cost = -1;
        std::string item;
    };

    struct answer
    {
        uint16_t    index = 0;
        std::string name;
    };

    struct event
    {
        std::string     type;
        uint16_t        map = 0;
        atlas::position at;
        std::string     detail;
        uint32_t        oid = 0;
        atlas::position goal;
        uint32_t        count = 0;
        uint16_t        dest  = 0;
    };

    struct pace
    {
        uint32_t                  count = 0;
        std::chrono::milliseconds total{0};
    };

    using death = game_bot::death_record;

    struct record
    {
        uint32_t                  failures = 0;
        std::string               reason;
        time_point                at;
        std::chrono::milliseconds duration{0};
        uint16_t                  map   = 0;
        uint8_t                   level = 0;
    };

    struct outcome
    {
        bool        succeeded = false;
        std::string reason;
    };

    struct concern
    {
        std::unordered_set<std::string> attacks;
        std::unordered_set<std::string> crew;
        uint32_t                        hp = 0;
    };

    struct mind
    {
        profile                                         traits;
        decision                                        current;
        ROLE                                            role = ROLE::SOLO;
        time_point                                      since;
        std::vector<transition>                         history;
        uint64_t                                        transitions = 0;
        uint64_t                                        session     = 0;
        std::unordered_map<std::string, fb::lua::value> memory;
        std::optional<death>                            died;
        std::unordered_map<std::string, pace>           paces;
        uint32_t                                        failures = 0;
        std::string                                     jumped;
        std::mt19937                                    random;
        std::string                                     performing;
        ACTION                                          acting = ACTION::TASK;
        std::vector<std::pair<std::string, uint32_t>>   missed;
        bool                                            interrupted = false;
        bool                                            alarming    = false;
        struct concern                                  concern;
        std::unordered_set<uint32_t>                    noticed;
        std::unordered_map<std::string, record>         records;
        std::chrono::milliseconds                       waited{0};
        std::chrono::milliseconds                       budget{0};
        std::vector<std::string>                        activities;
        std::optional<clock::time_point>                casted;
        uint32_t                                        closed = 0;
    };

private:
    std::unordered_map<fb::thread*, std::unique_ptr<fb::lua::root>>                    _scripts;
    mutable std::mutex                                                                 _mutex;
    std::unordered_map<uint32_t, mind>                                                 _minds;
    std::unordered_map<std::string, mind>                                              _archive;
    uint64_t                                                                           _sessions = 0;
    const bool                                                                         _recording;
    std::unordered_map<uint64_t, std::shared_ptr<async::task_completion_source<bool>>> _loops;
    boost::asio::thread_pool _workers{std::max(1u, std::thread::hardware_concurrency())};
    inline static std::mutex _ledger_mutex;
    inline static std::unordered_map<std::string, std::string>                          _ledger;
    inline static std::unordered_map<std::string, std::pair<uint16_t, double>>          _occupancy;
    inline static std::unordered_map<uint16_t, std::unordered_map<std::string, double>> _occupants;
    inline static std::unordered_map<uint32_t, std::unordered_map<uint16_t, double>>    _foreign_occupants;
    inline static std::unordered_map<std::string, listing>                              _roster;

public:
    explicit brain(bool recording = true);
    ~brain() = default;

public:
    void               attach(game_bot& bot, const profile& profile);
    void               persona(uint32_t bot, const std::unordered_map<std::string, double>& values);
    void               detach(game_bot& bot);
    void               transfer(game_bot& from, game_bot& to);
    async::task<void>  stop(game_bot& bot);
    bool               active(uint32_t bot) const;
    void               release();
    async::task<event> walk(std::shared_ptr<game_bot> bot, const destination& destination, const detour& detour);
    async::task<event> engage(std::shared_ptr<game_bot> bot, uint32_t oid, const maneuver& maneuver);
    async::task<bool>  synced(std::shared_ptr<game_bot> bot);
    async::task<event> talk(std::shared_ptr<game_bot>             bot,
                            std::shared_ptr<fb::protocol::header> opening,
                            std::vector<answer>                   answers);
    void               act(uint32_t bot, std::string_view activity);
    void               casting(uint32_t bot);
    void               close(game_bot& bot);
    bool               talking(const game_bot& bot) const;
    template <typename Fn>
    auto read(uint32_t bot, Fn&& fn) const -> std::optional<std::invoke_result_t<Fn, const mind&>>
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot);
        if (found == this->_minds.end())
            return std::nullopt;

        return fn(found->second);
    }
    template <typename T>
    async::task<T> offload(std::shared_ptr<game_bot> bot, std::function<T()> fn)
    {
        auto promise = std::make_shared<async::task_completion_source<T>>();
        boost::asio::post(this->_workers, [promise, fn = std::move(fn)]() {
            try
            {
                promise->set_value(fn());
            }
            catch (...)
            {
                promise->set_exception(std::current_exception());
            }
        });
        auto result = co_await promise->task();
        co_await bot->thread()->switching();
        co_return result;
    }
    void                       die(const game_bot& bot);
    void                       bury(game_bot& bot, const std::string& task);
    void                       tally(uint32_t bot, bool succeeded);
    void                       expect(uint32_t bot, std::chrono::milliseconds duration);
    bool                       stalled(uint32_t bot) const;
    void                       progress(uint32_t bot);
    void                       wait(uint32_t bot, std::chrono::milliseconds duration);
    void                       notice(game_bot& bot, std::string_view kind, uint32_t oid);
    void                       watch(uint32_t bot, const concern& value);
    void                       jumping(uint32_t bot, const std::string& means);
    std::optional<std::string> pick(uint32_t bot, const std::vector<std::pair<std::string, double>>& weights);
    static void                reserve(const std::string& name, const std::string& value);
    static void                release(const std::string& name);
    static void                occupy(const std::string& name, uint16_t map, double rate);
    static void                vacate(const std::string& name);
    static double              demand(uint16_t map, const std::unordered_set<std::string>& except);
    static std::unordered_map<uint16_t, double> occupied();
    static void occupied_elsewhere(uint32_t source, std::unordered_map<uint16_t, double> maps);
    static void post(const std::string& name, uint16_t map, fb::lua::value profile);
    static std::vector<std::pair<std::string, listing>> roster(const std::string& except);
    static std::unordered_map<std::string, uint32_t>    reservations(const std::string& except);
    void measure(uint32_t bot, const std::string& action, std::chrono::milliseconds elapsed);
    void remember(uint32_t bot, const std::string& key, const std::optional<fb::lua::value>& value);
    std::optional<fb::lua::value>                recall(uint32_t bot, const std::string& key) const;
    static snapshot                              observe(const game_bot& bot);
    static const char*                           name(STATE value);
    static const char*                           name(ROLE value);
    static const char*                           name(ACTION value);
    static const char*                           name(MANEUVER value);
    static void                                  bind(lua_State* L);
    static double                                seconds(time_point value);
    static std::string                           module(std::string_view task);
    static std::optional<STATE>                  parse(std::string_view value);
    static std::optional<game_bot::sight::entry> npc(const game_bot& bot, std::string_view name);

private:
    void                                 start(const std::shared_ptr<game_bot>& bot, uint64_t session);
    async::task<void>                    run(std::shared_ptr<game_bot> bot, uint64_t session);
    async::task<std::optional<decision>> think(std::shared_ptr<game_bot> bot);
    async::task<outcome>                 perform(std::shared_ptr<game_bot> bot, const decision& decision);
    void settle(const game_bot& bot, const std::string& task, const outcome& result, std::chrono::milliseconds elapsed);
    async::task<void> alarm(std::shared_ptr<game_bot> bot, std::string task, std::string kind);
    fb::lua::root&    scripts(game_bot& bot);
    bool              update(uint32_t bot, uint64_t session, const std::optional<decision>& next);
    bool              halted(uint32_t bot, const decision& decision) const;
    static void       evict(const std::string& name);
};

} // namespace fb::bot

#endif
