#include <fb/game/matchmaker.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>
#include <tuple>

using namespace fb::game;
using table = fb::model::table;

namespace mp      = fb::protocol::matchmaking;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

matchmaker::matchmaker(character& owner) :
    owner(owner)
{
    this->load({});
}

matchmaking_skill::dto_type matchmaking_skill::to_protocol(uint32_t user) const
{
    return dto_type{user, match_type, mu, sigma};
}

void matchmaker::load(const std::vector<matchmaking_skill::dto_type>& skills)
{
    this->_entries.clear();
    for (auto& skill : skills)
    {
        this->_entries.emplace(skill.match_type,
                               matchmaking_skill{
                                   .match_type = skill.match_type,
                                   .mu         = skill.mu,
                                   .sigma      = skill.sigma,
                               });
    }

    for (auto& [type, row] : table::matchmaking)
    {
        std::ignore = row;
        auto id     = static_cast<uint32_t>(type);
        if (this->_entries.contains(id) == false)
        {
            this->upsert(id, DEFAULT_MU, DEFAULT_SIGMA);
        }
    }
}

std::vector<matchmaking_skill::dto_type> matchmaker::to_protocol() const
{
    auto result = std::vector<matchmaking_skill::dto_type>{};
    result.reserve(this->_entries.size());
    for (auto& [match_type, skill] : this->_entries)
    {
        result.emplace_back(skill.to_protocol(this->owner.id));
    }

    return result;
}

std::optional<matchmaking_skill> matchmaker::get(uint32_t match_type) const
{
    auto i = this->_entries.find(match_type);
    if (i == this->_entries.end())
        return std::nullopt;

    return i->second;
}

void matchmaker::upsert(uint32_t match_type, double mu, double sigma)
{
    this->_entries.insert_or_assign(match_type,
                                    matchmaking_skill{
                                        .match_type = match_type,
                                        .mu         = mu,
                                        .sigma      = sigma,
                                    });
}

const std::optional<std::string>& matchmaker::pending_match_id() const
{
    return this->_pending_match_id;
}

void matchmaker::set_pending_match_id(std::string match_id)
{
    this->_pending_match_id = std::move(match_id);
}

void matchmaker::clear_pending_match_id()
{
    this->_pending_match_id = std::nullopt;
}

bool matchmaker::clear_pending_match_id_if(std::string_view match_id)
{
    if (!this->_pending_match_id.has_value() || this->_pending_match_id.value() != match_id)
        return false;

    this->_pending_match_id = std::nullopt;
    return true;
}

bool matchmaker::registered() const
{
    this->owner.assert_thread();

    return this->_registration.has_value() || this->_registering;
}

std::optional<std::string_view> matchmaker::registry_id() const
{
    this->owner.assert_thread();

    if (!this->_registration.has_value())
        return std::nullopt;

    return this->_registration->registry_id;
}

void matchmaker::begin_registration([[maybe_unused]] uint32_t match_type)
{
    this->owner.assert_thread();

    this->_registering = true;
}

void matchmaker::set_registration(uint32_t match_type, std::string registry_id)
{
    this->owner.assert_thread();

    this->_registering  = false;
    this->_registration = registration_state{
        .match_type  = match_type,
        .registry_id = std::move(registry_id),
    };
}

void matchmaker::clear_registration()
{
    this->owner.assert_thread();

    this->_registering  = false;
    this->_registration = std::nullopt;
}

void matchmaker::enqueue_squad_unregister(uint32_t match_type, std::string_view registry_id)
{
    this->owner.assert_thread();

    auto registry_id_str = std::string(registry_id);
    auto owner_id        = this->owner.id;
    this->owner.server.characters.foreach_enqueue(
        [registry_id_str, match_type](auto& ch) -> async::task<void> {
            auto id = ch->matchmaker.registry_id();
            if (id.has_value() == false || id.value() != registry_id_str)
                co_return;

            auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_unregister");
            if (lua)
            {
                lua->pushobject(ch);
                lua->pushinteger(match_type);
                lua->pushstring(registry_id_str.c_str());
                std::ignore = co_await lua->call(3);
            }
            ch->matchmaker.clear_pending_match_id();
            ch->matchmaker.clear_registration();
            co_return;
        },
        [owner_id](const character::container::character_ptr_t& ch) {
            return ch->id != owner_id;
        });
}

async::task<void> matchmaker::discard_leftover_registration()
{
    // Registration only lives for the duration of a session, so anything the queue
    // still holds at login time is leftover state that nobody can cancel.
    auto& server       = this->owner.server;
    auto  world        = this->owner.world();
    auto  character_id = this->owner.id;

    try
    {
        auto&& status =
            co_await server.http.post("matchmaking", "/matchmaking/status", mp_reqs::Status{world, character_id});

        if (status.error != 0 || status.registry_id.empty())
            co_return;

        auto&& resp =
            co_await server.http.post("matchmaking",
                                      "/matchmaking/unregister",
                                      mp_reqs::Unregister{status.match_type, status.registry_id, world, character_id});

        fb::logger::warn("matchmaking dropped leftover registry {} of character {} at login (success: {})",
                         status.registry_id,
                         character_id,
                         resp.success);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("matchmaking leftover check failed for character {}: {}", character_id, e.what());
    }
}

async::task<void> matchmaker::unregister_queue(bool quiet)
{
    this->owner.assert_thread();

    // Register HTTP still in flight — drop the local flag only.
    if (this->_registration.has_value() == false)
    {
        if (this->_registering)
            this->clear_registration();
        co_return;
    }

    auto registration = this->_registration.value();
    auto registry_id  = registration.registry_id;
    auto world        = this->owner.world();
    auto match_type   = registration.match_type;
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();
    this->clear_registration();

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp =
            co_await this->owner.server.http.post("matchmaking",
                                                  "/matchmaking/unregister",
                                                  mp_reqs::Unregister{match_type, registry_id, world, character_id});

        if (weak.lock() == nullptr)
            co_return;
        co_await this->owner.server.threads.switching(weak);

        if (resp.error != 0 || resp.success == false)
        {
            auto message = enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error));
            if (quiet)
                fb::logger::warn("matchmaking unregister failed for character {}: {}", character_id, message);
            else
                error = message;
        }
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        if (weak.lock() != nullptr)
            co_await this->owner.server.threads.switching(weak);

        if (quiet)
        {
            fb::logger::warn("matchmaking unregister failed for character {}: {}", character_id, error.value());
        }
        else
        {
            this->set_registration(match_type, registry_id);
            throw std::runtime_error(error.value());
        }
    }

    this->enqueue_squad_unregister(match_type, registry_id);

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_unregister");
        if (lua)
        {
            lua->pushobject(ptr);
            lua->pushinteger(match_type);
            lua->pushstring(registry_id.c_str());
            std::ignore = co_await lua->call(3);
        }
    }

    co_return;
}

async::task<void> matchmaker::confirm_queue(std::string match_id, bool quiet)
{
    this->owner.assert_thread();

    if (match_id.empty())
        co_return;

    auto world        = this->owner.world();
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp = co_await this->owner.server.http.post("matchmaking",
                                                            "/matchmaking/confirm",
                                                            mp_reqs::Confirm{match_id, world, character_id});

        if (weak.lock() == nullptr)
            co_return;
        co_await this->owner.server.threads.switching(weak);

        if (resp.error != 0)
        {
            auto message = enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error));
            if (quiet)
                fb::logger::warn("matchmaking confirm failed for character {}: {}", character_id, message);
            else
                error = message;
        }
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        if (weak.lock() != nullptr)
            co_await this->owner.server.threads.switching(weak);

        if (quiet)
            fb::logger::warn("matchmaking confirm failed for character {}: {}", character_id, error.value());
        else
            throw std::runtime_error(error.value());
    }

    if (!quiet)
    {
        auto match_type = this->_registration.has_value() ? this->_registration->match_type : 0;
        auto ptr        = weak.lock();
        if (ptr != nullptr)
        {
            auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_confirm");
            if (lua)
            {
                lua->pushobject(ptr);
                lua->pushstring(match_id.c_str());
                lua->pushinteger(match_type);
                std::ignore = co_await lua->call(3);
            }
        }
    }

    co_return;
}

async::task<void> matchmaker::decline_queue(std::string match_id, bool quiet)
{
    this->owner.assert_thread();

    if (match_id.empty())
        co_return;

    auto world        = this->owner.world();
    auto character_id = this->owner.id;
    auto weak         = this->owner.weak_from_this_as<character>();

    auto error = std::optional<std::string>{};
    try
    {
        auto&& resp = co_await this->owner.server.http.post("matchmaking",
                                                            "/matchmaking/decline",
                                                            mp_reqs::Decline{match_id, world, character_id});

        if (weak.lock() == nullptr)
            co_return;
        co_await this->owner.server.threads.switching(weak);

        if (resp.error != 0)
        {
            auto message = enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error));
            if (quiet)
                fb::logger::warn("matchmaking decline failed for character {}: {}", character_id, message);
            else
                error = message;
        }
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        if (weak.lock() != nullptr)
            co_await this->owner.server.threads.switching(weak);

        if (quiet)
            fb::logger::warn("matchmaking decline failed for character {}: {}", character_id, error.value());
        else
            throw std::runtime_error(error.value());
    }

    this->clear_pending_match_id_if(match_id);

    auto squad_notify = this->_registration;

    if (!quiet)
    {
        auto match_type = squad_notify.has_value() ? squad_notify->match_type : 0;
        auto ptr        = weak.lock();
        if (ptr != nullptr)
        {
            auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_decline");
            if (lua)
            {
                lua->pushobject(ptr);
                lua->pushstring(match_id.c_str());
                lua->pushinteger(match_type);
                std::ignore = co_await lua->call(3);
            }
        }
    }

    if (squad_notify.has_value())
    {
        this->clear_registration();
        this->enqueue_squad_unregister(squad_notify->match_type, squad_notify->registry_id);
    }

    co_return;
}

async::task<void> matchmaker::register_queue(uint32_t match_type)
{
    this->owner.assert_thread();

    if (!fb::config<std::optional<uint32_t>>("world"))
        throw std::runtime_error("교차 서버에서는 매치메이킹을 등록할 수 없습니다.");

    auto weak = this->owner.weak_from_this_as<character>();
    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    auto self = weak.lock();
    if (self == nullptr)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    auto match_type_enum = static_cast<fb::model::enum_value::MATCH_TYPE>(match_type);
    if (table::matchmaking->contains(match_type_enum) == false)
        throw std::runtime_error(enum_tostring(fb::model::enum_value::ERROR_CODE::MATCHMAKING_UNKNOWN_QUEUE));

    auto  matchmaking_table  = table::matchmaking;
    auto& matchmaking_config = matchmaking_table[match_type_enum];
    auto  world              = this->owner.world();
    auto  entries            = std::vector<mp::RegistryEntry>{};
    auto  participants       = std::vector<std::shared_ptr<character>>{};

    auto group_id = this->owner.group_id();
    if (group_id.has_value() == false)
    {
        participants.push_back(self);
    }
    else
    {
        auto group_guard = this->owner.server.groups.try_enter_read(group_id.value());
        if (group_guard.has_value() == false || group_guard->value() == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_JOINED));

        auto& group = *group_guard->value();
        if (group.master() != this->owner.name())
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_OWNER));

        auto member_names = group.members();
        participants.reserve(member_names.size());

        for (auto& name : member_names)
        {
            auto ch = this->owner.server.characters.find(name);
            if (ch == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_GROUP_CANNOT_FIND_TARGET));

            participants.push_back(ch);
        }
    }

    entries.reserve(participants.size());
    for (auto& ch : participants)
    {
        auto weak    = ch->weak_from_this_as<character>();
        auto builder = this->owner.server.threads.new_builder<mp::RegistryEntry>(weak);
        builder.func = [weak, world, match_type](auto&) -> async::task<mp::RegistryEntry> {
            auto ptr = weak.lock();
            if (ptr == nullptr)
                throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

            auto skill = ptr->matchmaker.get(match_type);
            auto mu    = skill.has_value() ? skill->mu : matchmaker::DEFAULT_MU;
            auto sigma = skill.has_value() ? skill->sigma : matchmaker::DEFAULT_SIGMA;
            co_return mp::RegistryEntry{world, ptr->id, mu, sigma};
        };
        entries.push_back(co_await builder.dispatch());
    }

    if (entries.size() > matchmaking_config.member_count)
        throw std::runtime_error(std::format("이 매치는 {}명까지 참가할 수 있습니다. (현재 그룹 {}명)",
                                             matchmaking_config.member_count,
                                             entries.size()));

    auto& server       = this->owner.server;
    auto  character_id = this->owner.id;

    // Mark registered before Register HTTP so Proposed cannot race ahead of local state.
    for (auto& ch : participants)
    {
        if (ch.get() == &this->owner)
        {
            this->begin_registration(match_type);
            continue;
        }

        auto member_weak = ch->weak_from_this_as<character>();
        auto builder     = this->owner.server.threads.new_builder(member_weak);
        builder.func     = [member_weak, match_type](auto&) -> async::task<void> {
            auto ptr = member_weak.lock();
            if (ptr == nullptr)
                co_return;
            ptr->matchmaker.begin_registration(match_type);
            co_return;
        };
        co_await builder.dispatch();
    }

    auto clear_in_flight = [this, &participants]() -> async::task<void> {
        for (auto& ch : participants)
        {
            if (ch.get() == &this->owner)
            {
                if (this->_registering && this->_registration.has_value() == false)
                    this->clear_registration();
                continue;
            }

            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = this->owner.server.threads.new_builder(member_weak);
            builder.func     = [member_weak](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;
                if (ptr->matchmaker.registry_id().has_value() == false)
                    ptr->matchmaker.clear_registration();
                co_return;
            };
            co_await builder.dispatch();
        }
        co_return;
    };

    mp_resp::Register resp;
    {
        auto http_error = std::optional<std::exception_ptr>{};
        try
        {
            resp = co_await server.http.post("matchmaking",
                                             "/matchmaking/register",
                                             mp_reqs::Register{match_type, entries});
        }
        catch (...)
        {
            // Do not co_await inside catch: throw; after suspend loses the exception.
            http_error = std::current_exception();
        }

        if (http_error.has_value())
        {
            if (weak.lock() != nullptr)
                co_await server.threads.switching(weak);
            co_await clear_in_flight();
            std::rethrow_exception(http_error.value());
        }
    }

    if (resp.error != 0)
    {
        if (weak.lock() != nullptr)
            co_await server.threads.switching(weak);
        co_await clear_in_flight();
        throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
    }

    self = weak.lock();
    if (self == nullptr)
    {
        // The character vanished while registering, so the queue would keep an owner-less registry.
        std::ignore = co_await server.http.post("matchmaking",
                                                "/matchmaking/unregister",
                                                mp_reqs::Unregister{match_type, resp.registry_id, world, character_id});
        // Do not touch this/owner — only clear participants still held by shared_ptr.
        for (auto& ch : participants)
        {
            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = server.threads.new_builder(member_weak);
            builder.func     = [member_weak](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;
                if (ptr->matchmaker.registry_id().has_value() == false)
                    ptr->matchmaker.clear_registration();
                co_return;
            };
            co_await builder.dispatch();
        }
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));
    }
    co_await server.threads.switching(weak);

    this->set_registration(match_type, resp.registry_id);

    {
        auto registry_id_str = resp.registry_id;
        for (auto& ch : participants)
        {
            if (ch.get() == &this->owner)
                continue;

            auto member_weak = ch->weak_from_this_as<character>();
            auto builder     = this->owner.server.threads.new_builder(member_weak);
            builder.func     = [member_weak, match_type, registry_id_str](auto&) -> async::task<void> {
                auto ptr = member_weak.lock();
                if (ptr == nullptr)
                    co_return;

                ptr->matchmaker.set_registration(match_type, registry_id_str);

                auto lua = ptr->server.lua.open("scripts/interaction.lua", "on_matchmaking_register");
                if (lua)
                {
                    lua->pushobject(ptr);
                    lua->pushinteger(match_type);
                    lua->pushstring(registry_id_str.c_str());
                    std::ignore = co_await lua->call(3);
                }
                co_return;
            };
            builder.enqueue();
        }
    }

    {
        auto lua = this->owner.server.lua.open("scripts/interaction.lua", "on_matchmaking_register");
        if (lua)
        {
            lua->pushobject(self);
            lua->pushinteger(match_type);
            lua->pushstring(resp.registry_id.c_str());
            std::ignore = co_await lua->call(3);
        }
    }

    co_return;
}
