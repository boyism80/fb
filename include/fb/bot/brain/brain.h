#ifndef __FB_BOT_BRAIN_BRAIN_H__
#define __FB_BOT_BRAIN_BRAIN_H__

#include <fb/bot/brain/atlas.h>
#include <fb/bot/game_bot.h>
#include <fb/lua.h>
#include <fb/model/model.h>

#include <async/task.h>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <thread>
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
        STATE       state = STATE::PREPARE;
        std::string task;
        uint8_t     step = 0;
        std::string reason;
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

    struct destination
    {
        std::unordered_set<uint16_t>   maps;
        std::optional<atlas::position> at;
        uint16_t                       range = 0;
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
        std::chrono::milliseconds    pace{0};
        std::vector<jump>            jumps;
        bool                         rush = false;
    };

    struct tactic
    {
        uint32_t                  guard = 0;
        std::chrono::milliseconds swing{0};
        std::chrono::milliseconds pace{0};
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
        uint32_t                        hp = 0;
    };

    struct mind
    {
        profile                                         traits;
        decision                                        current;
        ROLE                                            role = ROLE::SOLO;
        time_point                                      since;
        std::vector<transition>                         history;
        uint64_t                                        session = 0;
        std::unordered_map<std::string, fb::lua::value> memory;
        std::optional<death>                            died;
        std::unordered_map<std::string, pace>           paces;
        uint32_t                                        failures = 0;
        std::string                                     jumped;
        std::mt19937                                    random;
        std::string                                     performing;
        bool                                            interrupted = false;
        bool                                            alarming    = false;
        struct concern                                  concern;
        std::unordered_set<uint32_t>                    noticed;
        std::unordered_map<std::string, record>         records;
        std::chrono::milliseconds                       waited{0};
        std::chrono::milliseconds                       budget{0};
        std::vector<std::string>                        activities;
    };

private:
    std::unordered_map<fb::thread*, std::unique_ptr<fb::lua::root>> _scripts;
    mutable std::mutex                                              _mutex;
    std::unordered_map<uint32_t, mind>                              _minds;
    std::unordered_map<std::string, mind>                           _archive;
    uint64_t                                                        _sessions = 0;
    std::unordered_set<uint64_t>                                    _loops;
    boost::asio::thread_pool _workers{std::max(1u, std::thread::hardware_concurrency())};
    inline static std::mutex _ledger_mutex;
    inline static std::unordered_map<std::string, std::string> _ledger;

public:
    brain()  = default;
    ~brain() = default;

public:
    void                attach(game_bot& bot, const profile& profile);
    void                persona(uint32_t bot, const std::unordered_map<std::string, double>& values);
    void                detach(game_bot& bot);
    void                transfer(game_bot& from, game_bot& to);
    async::task<void>   stop(game_bot& bot);
    bool                active(uint32_t bot) const;
    void                release();
    std::optional<mind> find(uint32_t bot) const;
    async::task<event>  walk(std::shared_ptr<game_bot> bot, const destination& destination, const detour& detour);
    async::task<event>  attack(std::shared_ptr<game_bot> bot, uint32_t oid, const tactic& tactic);
    void                act(uint32_t bot, std::string_view activity);
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
    void                       bury(game_bot& bot);
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
    static std::unordered_map<std::string, uint32_t> reservations(const std::string& except);
    void measure(uint32_t bot, const std::string& action, std::chrono::milliseconds elapsed);
    void remember(uint32_t bot, const std::string& key, const std::optional<fb::lua::value>& value);
    std::optional<fb::lua::value>                recall(uint32_t bot, const std::string& key) const;
    static snapshot                              observe(const game_bot& bot);
    static const char*                           name(STATE value);
    static const char*                           name(ROLE value);
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
};

} // namespace fb::bot

#endif
