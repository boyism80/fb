#include <fb/bot/brain/brain.h>

#include <fb/bot/builtin/integration_test.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/integration/lua_integration_model.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/logger.h>
#include <fb/thread.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <chrono>
#include <string>
#include <string_view>

using namespace fb::bot;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;
using namespace std::chrono_literals;

constexpr auto RETRY_FLOOR     = 100ms;
constexpr auto STALL_FACTOR    = 2;
constexpr auto ALERT_MODULE    = "brain.policies.alert";
constexpr auto DECIDE_MODULE   = "brain.decide";
constexpr auto SKILLS_MODULE   = "brain.skills";
constexpr auto OBSERVE_TIMEOUT = 5s;

void brain::attach(game_bot& bot, const profile& profile)
{
    auto self    = this->shared_from_this_as<brain>();
    auto shared  = bot.shared_from_this_as<game_bot>();
    auto session = uint64_t{0};
    {
        auto lock     = std::lock_guard(this->_mutex);
        session       = ++this->_sessions;
        auto& found   = this->_minds[bot.id];
        found         = mind{};
        auto archived = this->_archive.find(bot.name());
        if (archived != this->_archive.end())
        {
            found.memory  = std::move(archived->second.memory);
            found.records = std::move(archived->second.records);
            found.paces   = std::move(archived->second.paces);
            found.died    = std::move(archived->second.died);
            this->_archive.erase(archived);
        }
        found.traits  = profile;
        found.session = session;
        for (auto& [key, value] : profile.memory)
        {
            found.memory.insert_or_assign(key, value);
        }
        if (bot.last_death().has_value())
            found.died = bot.last_death();
        found.random.seed(profile.seed != 0 ? profile.seed : bot.id);
    }
    this->close(bot);
    bot.attached(self);
    this->start(shared, session);
}

void brain::transfer(game_bot& from, game_bot& to)
{
    auto self    = this->shared_from_this_as<brain>();
    auto shared  = to.shared_from_this_as<game_bot>();
    auto session = uint64_t{0};
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(from.id);
        if (found == this->_minds.end())
            return;

        auto moved    = std::move(found->second);
        session       = ++this->_sessions;
        moved.session = session;
        this->_minds.erase(found);
        this->_minds.insert_or_assign(to.id, std::move(moved));
    }
    this->close(to);
    if (from.attached().get() == this)
        from.attached(nullptr);
    to.attached(self);
    this->start(shared, session);
}

void brain::close(game_bot& bot)
{
    auto menu  = std::optional<integration::dialog_bot>{};
    auto ext   = std::optional<integration::dialog_ext_bot>{};
    auto seq   = bot.last_dialog(menu, ext);
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot.id);
    if (found != this->_minds.end())
        found->second.closed = seq;
}

bool brain::talking(const game_bot& bot) const
{
    auto menu  = std::optional<integration::dialog_bot>{};
    auto ext   = std::optional<integration::dialog_ext_bot>{};
    auto seq   = bot.last_dialog(menu, ext);
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot.id);
    return found != this->_minds.end() && seq != found->second.closed;
}

async::task<void> brain::stop(game_bot& bot)
{
    auto finished = std::shared_ptr<async::task_completion_source<bool>>{};
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot.id);
        if (found != this->_minds.end())
        {
            auto loop = this->_loops.find(found->second.session);
            if (loop != this->_loops.end())
                finished = loop->second;
        }
    }
    this->detach(bot);
    if (finished == nullptr)
        co_return;

    bot.wake();
    std::ignore = co_await finished->task();
}

bool brain::active(uint32_t bot) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    return found != this->_minds.end() && found->second.interrupted == false;
}

void brain::start(const std::shared_ptr<game_bot>& bot, uint64_t session)
{
    fb::logger::debug("brain: start bot_id={} session={}", bot->id, session);
    {
        auto lock = std::lock_guard(this->_mutex);
        this->_loops.insert_or_assign(session, std::make_shared<async::task_completion_source<bool>>());
    }
    auto self    = this->shared_from_this_as<brain>();
    auto builder = bot->thread()->new_builder<void>();
    builder.func = [self, bot, session](auto&) -> async::task<void> {
        co_await self->run(bot, session);
    };
    builder.enqueue();
}

void brain::detach(game_bot& bot)
{
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot.id);
        if (found != this->_minds.end())
        {
            this->_archive.insert_or_assign(bot.name(), std::move(found->second));
            this->_minds.erase(found);
        }
    }
    brain::vacate(bot.name());
    if (bot.attached().get() == this)
        bot.attached(nullptr);
}

void brain::persona(uint32_t bot, const std::unordered_map<std::string, double>& values)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    for (auto& [key, value] : values)
    {
        found->second.traits.persona.insert_or_assign(key, value);
    }
}

void brain::release()
{
    auto lock = std::lock_guard(this->_mutex);
    this->_minds.clear();
}

std::optional<brain::mind> brain::find(uint32_t bot) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return std::nullopt;

    return found->second;
}

brain::snapshot brain::observe(const game_bot& bot)
{
    auto result   = snapshot{};
    result.level  = bot.level();
    result.ghost  = bot.state() == fb::model::enum_value::STATE::GHOST;
    result.map    = bot.map();
    result.nation = bot.nation();
    if (fb::model::table::promotion->name2class(bot.class_name(), result.cls, result.promotion) == false)
    {
        result.cls       = fb::model::enum_value::CLASS::NONE;
        result.promotion = 0;
    }
    return result;
}

const char* brain::name(STATE value)
{
    switch (value)
    {
    case STATE::DEAD:
        return "DEAD";
    case STATE::PREPARE:
        return "PREPARE";
    case STATE::TRAVEL:
        return "TRAVEL";
    case STATE::HUNT:
        return "HUNT";
    case STATE::RETURN:
        return "RETURN";
    case STATE::PROGRESS:
        return "PROGRESS";
    }
    return "";
}

const char* brain::name(ROLE value)
{
    switch (value)
    {
    case ROLE::SOLO:
        return "SOLO";
    case ROLE::LEADER:
        return "LEADER";
    case ROLE::MEMBER:
        return "MEMBER";
    }
    return "";
}

const char* brain::name(NODE value)
{
    switch (value)
    {
    case NODE::SELECTOR:
        return "SELECTOR";
    case NODE::SEQUENCE:
        return "SEQUENCE";
    case NODE::CHECK:
        return "CHECK";
    case NODE::HEAL:
        return "HEAL";
    case NODE::POTION:
        return "POTION";
    case NODE::HURT:
        return "HURT";
    case NODE::BUFF:
        return "BUFF";
    case NODE::SPELL:
        return "SPELL";
    case NODE::MANA:
        return "MANA";
    case NODE::APPROACH:
        return "APPROACH";
    case NODE::FACE:
        return "FACE";
    case NODE::OPENER:
        return "OPENER";
    case NODE::SWING:
        return "SWING";
    }
    return "";
}

const char* brain::name(ACTION value)
{
    switch (value)
    {
    case ACTION::TASK:
        return "TASK";
    case ACTION::CONTINUE:
        return "CONTINUE";
    case ACTION::HOLD:
        return "HOLD";
    case ACTION::MOVE:
        return "MOVE";
    case ACTION::ATTACK:
        return "ATTACK";
    case ACTION::TALK:
        return "TALK";
    case ACTION::READ:
        return "READ";
    case ACTION::USE:
        return "USE";
    case ACTION::PICK:
        return "PICK";
    case ACTION::EQUIP:
        return "EQUIP";
    case ACTION::WAIT:
        return "WAIT";
    case ACTION::FORGET:
        return "FORGET";
    case ACTION::CAST:
        return "CAST";
    case ACTION::SAY:
        return "SAY";
    }
    return "";
}

void brain::bind(lua_State* L)
{
    lua_newtable(L);
    for (auto value : {ACTION::TASK,
                       ACTION::CONTINUE,
                       ACTION::HOLD,
                       ACTION::MOVE,
                       ACTION::ATTACK,
                       ACTION::TALK,
                       ACTION::READ,
                       ACTION::USE,
                       ACTION::PICK,
                       ACTION::EQUIP,
                       ACTION::WAIT,
                       ACTION::FORGET,
                       ACTION::CAST,
                       ACTION::SAY})
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        lua_setfield(L, -2, brain::name(value));
    }
    lua_setglobal(L, "ACTION");

    lua_newtable(L);
    for (auto value : {NODE::SELECTOR,
                       NODE::SEQUENCE,
                       NODE::CHECK,
                       NODE::HEAL,
                       NODE::POTION,
                       NODE::HURT,
                       NODE::BUFF,
                       NODE::SPELL,
                       NODE::MANA,
                       NODE::APPROACH,
                       NODE::FACE,
                       NODE::OPENER,
                       NODE::SWING})
    {
        lua_pushinteger(L, static_cast<lua_Integer>(value));
        lua_setfield(L, -2, brain::name(value));
    }
    lua_setglobal(L, "NODE");
}

fb::lua::root& brain::scripts(game_bot& bot)
{
    auto thread = bot.thread();
    auto lock   = std::lock_guard(this->_mutex);
    auto found  = this->_scripts.find(thread);
    if (found != this->_scripts.end())
        return *found->second;

    auto  created = std::make_unique<fb::lua::root>(bot.controller.container, *thread);
    auto& lua     = *created;
    lua.build<fb::lua::luable>();
    lua.build<game_bot, fb::lua::luable>();
    lua.build<brain, fb::lua::luable>();
    integration::lua_protocol::register_metatables(static_cast<lua_State*>(lua));
    integration::lua_protocol::register_modules(lua);
    integration::register_model_globals(lua);
    brain::bind(static_cast<lua_State*>(lua));
    lua.build("log", builtin::integration_test::builtin_log);
    lua.package_path("../../scripts/?.lua");
    lua.package_path("../../scripts/?/init.lua");
    lua.package_path("scripts/?.lua");
    lua.package_path("scripts/?/init.lua");
    return *this->_scripts.insert_or_assign(thread, std::move(created)).first->second;
}

std::string brain::module(std::string_view task)
{
    auto result = std::string{"brain.tasks."};
    std::transform(task.begin(), task.end(), std::back_inserter(result), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::optional<brain::STATE> brain::parse(std::string_view value)
{
    for (auto candidate : {STATE::DEAD, STATE::PREPARE, STATE::TRAVEL, STATE::HUNT, STATE::RETURN, STATE::PROGRESS})
    {
        if (value == brain::name(candidate))
            return candidate;
    }
    return std::nullopt;
}

std::optional<game_bot::sight::entry> brain::npc(const game_bot& bot, std::string_view name)
{
    auto model = fb::model::table::npc->name2npc(name);
    if (model == nullptr)
        return std::nullopt;

    auto spawns = fb::model::table::npc_spawn->find(bot.map());
    if (spawns == nullptr)
        return std::nullopt;

    for (auto& spawn : *spawns)
    {
        if (spawn.npc != model->id)
            continue;

        for (auto& [_, entry] : bot.sight.entries())
        {
            if (entry.position == spawn.position)
                return entry;
        }
    }
    return std::nullopt;
}

double brain::seconds(time_point value)
{
    return std::chrono::duration<double>(value.time_since_epoch()).count();
}

async::task<void> brain::run(std::shared_ptr<game_bot> bot, uint64_t session)
{
    [[maybe_unused]] auto self = this->shared_from_this_as<brain>();
    while (bot->aborted() == false)
    {
        co_await bot->yield();
        if (bot->inited() == false || bot->map() == 0xFFFF)
        {
            std::ignore = co_await bot->changed();
            continue;
        }

        if (bot->level() > 0 && bot->class_name().empty())
        {
            try
            {
                std::ignore = co_await bot->request_by_opcode(
                    game_resp::internal_info<fb::protocol::CLIENT_VERSION::v550>::opcode,
                    bot->make<game_reqs::self_info>().get(),
                    [](const fb::protocol::header&) {
                        return true;
                    },
                    OBSERVE_TIMEOUT);
            }
            catch (const std::exception&)
            { }
        }

        auto next = bot->level() > 0 && bot->class_name().empty() == false ? co_await this->think(bot) : std::nullopt;
        if (this->update(bot->id, session, next) == false)
            break;

        auto progressed = false;
        if (next.has_value() && next->task.empty() == false && next->kind != ACTION::CONTINUE &&
            next->kind != ACTION::HOLD && this->halted(bot->id, next.value()) == false)
        {
            {
                auto lock  = std::lock_guard(this->_mutex);
                auto found = this->_minds.find(bot->id);
                if (found != this->_minds.end())
                    found->second.performing = next->task;
            }

            bot->interruptible(true);
            auto started = clock::now();
            auto result  = co_await this->perform(bot, next.value());
            bot->interruptible(false);
            auto interrupted = false;
            {
                auto lock  = std::lock_guard(this->_mutex);
                auto found = this->_minds.find(bot->id);
                if (found != this->_minds.end())
                {
                    interrupted               = found->second.interrupted;
                    found->second.interrupted = false;
                    found->second.performing.clear();
                }
            }

            progressed = interrupted || result.succeeded;
            if (interrupted == false)
            {
                this->tally(bot->id, result.succeeded);
                this->settle(*bot,
                             next->task,
                             result,
                             std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
            }
        }

        if (progressed == false)
            std::ignore = co_await bot->changed(next.has_value() ? next->revisit : std::chrono::milliseconds{0});
    }

    auto finished = std::shared_ptr<async::task_completion_source<bool>>{};
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_loops.find(session);
        if (found != this->_loops.end())
        {
            finished = found->second;
            this->_loops.erase(found);
        }
    }
    if (finished != nullptr)
    {
        auto builder = bot->thread()->new_builder<void>();
        builder.func = [finished](auto&) -> async::task<void> {
            auto failure = std::exception_ptr{};
            std::ignore  = finished->try_set_value(true, failure);
            co_return;
        };
        builder.enqueue();
    }
}

async::task<bool> brain::synced(std::shared_ptr<game_bot> bot)
{
    try
    {
        std::ignore = co_await bot->request_by_opcode(
            game_resp::internal_info<fb::protocol::CLIENT_VERSION::v550>::opcode,
            bot->make<game_reqs::self_info>().get(),
            [](const fb::protocol::header&) {
                return true;
            },
            OBSERVE_TIMEOUT);
        co_return true;
    }
    catch (const std::exception&)
    {
        co_return false;
    }
}

void brain::die(const game_bot& bot)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot.id);
    if (found != this->_minds.end())
        found->second.died = bot.last_death();
}

std::optional<std::string> brain::pick(uint32_t bot, const std::vector<std::pair<std::string, double>>& weights)
{
    auto values = std::vector<double>{};
    for (auto& [_, weight] : weights)
    {
        values.push_back(std::max(0.0, weight));
    }
    if (std::none_of(values.begin(), values.end(), [](auto value) {
            return value > 0.0;
        }))
        return std::nullopt;

    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return std::nullopt;

    auto index = std::discrete_distribution<size_t>(values.begin(), values.end())(found->second.random);
    return weights.at(index).first;
}

void brain::reserve(const std::string& name, const std::string& value)
{
    auto lock = std::lock_guard(_ledger_mutex);
    _ledger.insert_or_assign(name, value);
}

void brain::release(const std::string& name)
{
    auto lock = std::lock_guard(_ledger_mutex);
    _ledger.erase(name);
}

void brain::occupy(const std::string& name, uint16_t map, double rate)
{
    auto lock = std::lock_guard(_ledger_mutex);
    _occupancy.insert_or_assign(name, std::pair{map, rate});
}

void brain::vacate(const std::string& name)
{
    auto lock = std::lock_guard(_ledger_mutex);
    _occupancy.erase(name);
}

double brain::demand(uint16_t map, const std::string& except)
{
    auto lock  = std::lock_guard(_ledger_mutex);
    auto total = 0.0;
    for (auto& [name, entry] : _occupancy)
    {
        if (name != except && entry.first == map)
            total += entry.second;
    }
    return total;
}

std::unordered_map<std::string, uint32_t> brain::reservations(const std::string& except)
{
    auto lock   = std::lock_guard(_ledger_mutex);
    auto result = std::unordered_map<std::string, uint32_t>{};
    for (auto& [name, value] : _ledger)
    {
        if (name != except)
            result[value]++;
    }
    return result;
}

void brain::jumping(uint32_t bot, const std::string& means)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found != this->_minds.end())
        found->second.jumped = means;
}

void brain::watch(uint32_t bot, const concern& value)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found != this->_minds.end())
        found->second.concern = value;
}

void brain::notice(game_bot& bot, std::string_view kind, uint32_t oid)
{
    auto attack = std::string{};
    if (kind == "sight")
    {
        auto entry = bot.sight.entries().find(oid);
        auto mob   = entry != bot.sight.entries().end()
                         ? atlas::get().mob(bot.map(), entry->second.look, entry->second.color)
                         : std::nullopt;
        auto model = mob.has_value() ? fb::model::table::mob->find(mob.value()) : nullptr;
        if (model == nullptr)
            return;
        attack = fb::model::enum_value::enum_tostring(model->attack_type);
    }

    auto task = std::string{};
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot.id);
        if (found == this->_minds.end() || found->second.performing.empty() || found->second.interrupted ||
            found->second.alarming)
            return;

        auto& mind = found->second;
        if (kind == "hit" && bot.hp() > mind.concern.hp)
            return;

        if (kind == "sight")
        {
            std::erase_if(mind.noticed, [&bot](uint32_t seen) {
                return bot.sight.entries().contains(seen) == false;
            });
            if (mind.concern.attacks.contains(attack) == false || mind.noticed.insert(oid).second == false)
                return;
        }

        mind.alarming = true;
        task          = mind.performing;
    }

    async::awaitable_then(this->alarm(bot.shared_from_this_as<game_bot>(), task, std::string(kind)), [](auto result) {
        try
        {
            result();
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("brain: alarm failed: {}", e.what());
        }
    });
}

async::task<void> brain::alarm(std::shared_ptr<game_bot> bot, std::string task, std::string kind)
{
    auto  reason = std::string{};
    auto  opts   = fb::lua::call_options{.auto_resume_parent = false};
    auto* raw    = this->scripts(*bot).pop(nullptr, opts);
    if (raw != nullptr)
    {
        auto ctx = fb::lua::context::guard{raw};
        auto L   = static_cast<lua_State*>(*ctx);
        lua_getglobal(L, "require");
        lua_pushstring(L, ALERT_MODULE);
        if (lua_pcall(L, 1, 1, 0) == LUA_OK)
        {
            lua_getfield(L, -1, "reason");
            ctx->pushobject(this->shared_from_this_as<brain>());
            ctx->pushobject(bot);
            lua_pushstring(L, task.c_str());
            lua_pushstring(L, kind.c_str());
            try
            {
                auto base = lua_gettop(L) - 5;
                if (co_await ctx->call(4) && lua_gettop(L) > base && lua_isstring(L, base + 1))
                    reason = lua_tostring(L, base + 1);
            }
            catch (const std::exception& e)
            {
                fb::logger::fatal("brain: bot={} alert failed: {}", bot->name(), e.what());
            }
        }
        else
        {
            fb::logger::fatal("brain: bot={} cannot load {}: {}", bot->name(), ALERT_MODULE, lua_tostring(L, -1));
        }
    }

    auto interrupt = false;
    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot->id);
        if (found != this->_minds.end())
        {
            found->second.alarming = false;
            interrupt              = reason.empty() == false && found->second.performing == task;
            if (interrupt)
                found->second.interrupted = true;
        }
    }

    if (interrupt)
    {
        fb::logger::debug("brain: bot_id={} interrupt {} by {} ({})", bot->id, task, reason, kind);
        bot->interrupt_hooks();
    }
}

void brain::tally(uint32_t bot, bool succeeded)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    found->second.failures = succeeded ? 0 : found->second.failures + 1;
}

void brain::measure(uint32_t bot, const std::string& action, std::chrono::milliseconds elapsed)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    auto& pace = found->second.paces[action];
    pace.count++;
    pace.total += elapsed;
}

void brain::bury(game_bot& bot)
{
    bot.forget_death();
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot.id);
    if (found != this->_minds.end())
        found->second.died.reset();
}

void brain::remember(uint32_t bot, const std::string& key, const std::optional<fb::lua::value>& value)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    if (value.has_value())
        found->second.memory.insert_or_assign(key, value.value());
    else
        found->second.memory.erase(key);
}

std::optional<fb::lua::value> brain::recall(uint32_t bot, const std::string& key) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return std::nullopt;

    auto entry = found->second.memory.find(key);
    if (entry == found->second.memory.end())
        return std::nullopt;
    return entry->second;
}

async::task<std::optional<brain::decision>> brain::think(std::shared_ptr<game_bot> bot)
{
    auto  opts = fb::lua::call_options{.auto_resume_parent = false};
    auto* raw  = this->scripts(*bot).pop(nullptr, opts);
    if (raw == nullptr)
        co_return std::nullopt;

    auto ctx = fb::lua::context::guard{raw};
    auto L   = static_cast<lua_State*>(*ctx);
    lua_getglobal(L, "require");
    lua_pushstring(L, DECIDE_MODULE);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK)
    {
        fb::logger::fatal("brain: bot={} cannot load {}: {}", bot->name(), DECIDE_MODULE, lua_tostring(L, -1));
        this->tally(bot->id, false);
        co_return std::nullopt;
    }

    lua_getfield(L, -1, "decide");
    ctx->pushobject(this->shared_from_this_as<brain>());
    ctx->pushobject(bot);

    auto result = std::optional<decision>{};
    try
    {
        auto base = lua_gettop(L) - 3;
        if (co_await ctx->call(2) && lua_gettop(L) > base && lua_istable(L, base + 1))
        {
            lua_pushvalue(L, base + 1);
            auto next = decision{};
            lua_getfield(L, -1, "state");
            auto state = brain::parse(ctx->tostring(-1));
            lua_pop(L, 1);

            lua_getfield(L, -1, "task");
            next.task = ctx->tostring(-1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "step");
            next.step = static_cast<uint8_t>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            lua_getfield(L, -1, "reason");
            next.reason = ctx->tostring(-1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "kind");
            next.kind = static_cast<ACTION>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            lua_getfield(L, -1, "revisit");
            next.revisit = std::chrono::milliseconds(static_cast<int64_t>(std::ceil(lua_tonumber(L, -1))));
            lua_pop(L, 1);
            next.action = fb::lua::value::copy_data(L, -1);

            if (state.has_value())
            {
                next.state = state.value();
                result     = next;
            }
            else
            {
                fb::logger::fatal("brain: bot={} decide returned an unknown state", bot->name());
                this->tally(bot->id, false);
            }
        }
        else
        {
            fb::logger::fatal("brain: bot={} decide returned no decision", bot->name());
            this->tally(bot->id, false);
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("brain: bot={} decide failed: {}", bot->name(), e.what());
        this->tally(bot->id, false);
    }
    co_return result;
}

void brain::settle(const game_bot&           bot,
                   const std::string&        task,
                   const outcome&            result,
                   std::chrono::milliseconds elapsed)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot.id);
    if (found == this->_minds.end())
        return;

    auto& records = found->second.records;
    if (result.succeeded)
    {
        records.erase(task);
        return;
    }

    auto& entry    = records[task];
    entry.failures = entry.failures + 1;
    fb::logger::debug("brain: bot_id={} {} failed ({}) x{}", bot.id, task, result.reason, entry.failures);
    entry.reason   = result.reason;
    entry.at       = clock::now();
    entry.duration = std::max(elapsed, std::chrono::duration_cast<std::chrono::milliseconds>(RETRY_FLOOR));
    entry.map      = bot.map();
    entry.level    = bot.level();
}

void brain::expect(uint32_t bot, std::chrono::milliseconds duration)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    found->second.budget = duration;
    found->second.waited = std::chrono::milliseconds{0};
}

void brain::wait(uint32_t bot, std::chrono::milliseconds duration)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found != this->_minds.end())
        found->second.waited += duration;
}

void brain::progress(uint32_t bot)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found != this->_minds.end())
        found->second.waited = std::chrono::milliseconds{0};
}

bool brain::stalled(uint32_t bot) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end() || found->second.budget <= std::chrono::milliseconds{0})
        return false;

    auto  budget = found->second.budget;
    auto& paces  = found->second.paces;
    auto  hop    = paces.find("hop");
    if (hop != paces.end() && hop->second.count > 0)
        budget = std::max(budget, hop->second.total / hop->second.count);
    return found->second.waited > budget * STALL_FACTOR;
}

async::task<brain::outcome> brain::perform(std::shared_ptr<game_bot> bot, const decision& decision)
{
    auto  opts = fb::lua::call_options{.auto_resume_parent = false};
    auto* raw  = this->scripts(*bot).pop(nullptr, opts);
    if (raw == nullptr)
        co_return outcome{false, "NO_CONTEXT"};

    {
        auto lock  = std::lock_guard(this->_mutex);
        auto found = this->_minds.find(bot->id);
        if (found != this->_minds.end())
            found->second.budget = std::chrono::milliseconds{0};
    }

    auto ctx    = fb::lua::context::guard{raw};
    auto L      = static_cast<lua_State*>(*ctx);
    auto module = decision.kind == ACTION::TASK ? brain::module(decision.task) : std::string{SKILLS_MODULE};
    lua_getglobal(L, "require");
    lua_pushstring(L, module.c_str());
    if (lua_pcall(L, 1, 1, 0) != LUA_OK)
    {
        fb::logger::fatal("brain: bot={} cannot load {}: {}", bot->name(), module, lua_tostring(L, -1));
        co_return outcome{false, "NO_MODULE"};
    }

    lua_getfield(L, -1, "run");
    ctx->pushobject(this->shared_from_this_as<brain>());
    ctx->pushobject(bot);
    if (decision.kind == ACTION::TASK)
        lua_pushinteger(L, decision.step);
    else
        decision.action.push(*ctx);

    auto result = outcome{};
    try
    {
        auto base = lua_gettop(L) - 4;
        if (co_await ctx->call(3) && lua_gettop(L) > base)
        {
            result.succeeded = lua_toboolean(L, base + 1) != 0;
            result.reason    = lua_gettop(L) > base + 1 && lua_isstring(L, base + 2) ? lua_tostring(L, base + 2) : "";
        }
    }
    catch (const std::exception& e)
    {
        result.reason = "ERROR";
        fb::logger::fatal("brain: bot={} task {} step {} failed: {}",
                          bot->name(),
                          decision.task,
                          decision.step,
                          e.what());
    }
    co_return result;
}

bool brain::update(uint32_t bot, uint64_t session, const std::optional<decision>& next)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end() || found->second.session != session)
        return false;

    if (next.has_value())
    {
        auto& mind    = found->second;
        auto  changed = mind.history.empty() || mind.current.state != next->state || mind.current.task != next->task ||
                       mind.current.step != next->step || mind.current.kind != next->kind;
        if (changed)
        {
            auto now     = clock::now();
            mind.current = next.value();
            mind.since   = now;
            mind.history.push_back(transition{next.value(), now});
            fb::logger::debug("brain: bot_id={} {}/{}/{} {} ({})",
                              bot,
                              brain::name(next->state),
                              next->task,
                              next->step,
                              brain::name(next->kind),
                              next->reason);
        }
    }
    return true;
}

bool brain::halted(uint32_t bot, const decision& decision) const
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return true;

    auto& halt = found->second.traits.halt;
    return halt.has_value() && halt->task == decision.task && decision.step >= halt->step;
}
