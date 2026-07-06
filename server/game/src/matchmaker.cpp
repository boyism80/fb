#include <fb/game/matchmaker.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <macro.h>

using namespace fb::game;
using table = fb::model::table;

namespace mp      = fb::protocol::matchmaking;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

matchmaker::matchmaker(character& owner) :
    owner(owner)
{ }

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

matchmaking_skill matchmaker::ensure_skill(uint32_t match_type)
{
    this->owner.assert_thread();

    auto skill = this->get(match_type);
    if (skill.has_value())
        return skill.value();

    this->upsert(match_type, DEFAULT_MU, DEFAULT_SIGMA);
    return matchmaking_skill{
        .match_type = match_type,
        .mu         = DEFAULT_MU,
        .sigma      = DEFAULT_SIGMA,
    };
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

bool matchmaker::enrolled() const
{
    return this->_enrollment.has_value();
}

std::optional<std::string_view> matchmaker::registry_id() const
{
    if (!this->_enrollment.has_value())
        return std::nullopt;

    return this->_enrollment->registry_id;
}

void matchmaker::set_enrollment(uint32_t match_type, std::string registry_id)
{
    this->_enrollment = enrollment_state{
        .match_type  = match_type,
        .registry_id = std::move(registry_id),
    };
}

void matchmaker::clear_enrollment()
{
    this->_enrollment = std::nullopt;
}

async::task<void> matchmaker::unregister_queue(bool quiet)
{
    this->owner.assert_thread();

    if (!this->_enrollment.has_value())
        co_return;

    auto  enrollment   = this->_enrollment.value();
    auto  registry_id  = enrollment.registry_id;
    auto  world        = fb::config<uint32_t>("world");
    auto& match_type   = enrollment.match_type;
    auto  character_id = this->owner.id;

    try
    {
        auto&& resp =
            co_await this->owner.server.http.post("matchmaking",
                                                  "/matchmaking/unregister",
                                                  mp_reqs::Unregister{match_type, registry_id, world, character_id});

        if (resp.error != 0 || resp.success == false)
        {
            if (quiet)
            {
                fb::logger::warn("matchmaking unregister failed for character {}: {}",
                                 character_id,
                                 enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
            else
            {
                throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
        }
    }
    catch (const std::exception& e)
    {
        if (quiet)
            fb::logger::warn("matchmaking unregister failed for character {}: {}", character_id, e.what());
        else
            throw;
    }

    {
        auto registry_id_str = registry_id;
        auto owner_id        = this->owner.id;
        auto guard           = this->owner.server.characters.enter_read();
        guard.value().foreach_enqueue(
            [registry_id_str, match_type](auto& ch) -> async::task<void> {
                auto lua = ch->server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_unregister");
                if (lua)
                {
                    lua->pushobject(ch);
                    lua->pushinteger(match_type);
                    lua->pushstring(registry_id_str.c_str());
                    std::ignore = lua->call(3);
                }
                ch->matchmaker.clear_enrollment();
                co_return;
            },
            [registry_id_str, owner_id](const character::container::character_ptr_t& ch) {
                if (ch->id == owner_id)
                    return false;

                auto id = ch->matchmaker.registry_id();
                return id.has_value() && id.value() == registry_id_str;
            });
    }

    {
        auto ptr = this->owner.weak_from_this_as<character>().lock();
        if (ptr != nullptr)
        {
            auto lua = this->owner.server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_unregister");
            if (lua)
            {
                lua->pushobject(ptr);
                lua->pushinteger(match_type);
                lua->pushstring(registry_id.c_str());
                std::ignore = lua->call(3);
            }
        }
    }

    this->clear_enrollment();
    co_return;
}

async::task<void> matchmaker::confirm_queue(std::string_view match_id, bool quiet)
{
    this->owner.assert_thread();

    if (match_id.empty())
        co_return;

    auto world        = fb::config<uint32_t>("world");
    auto character_id = this->owner.id;

    try
    {
        auto&& resp = co_await this->owner.server.http.post("matchmaking",
                                                            "/matchmaking/confirm",
                                                            mp_reqs::Confirm{match_id, world, character_id});

        if (resp.error != 0)
        {
            if (quiet)
            {
                fb::logger::warn("matchmaking confirm failed for character {}: {}",
                                 character_id,
                                 enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
            else
            {
                throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
        }
    }
    catch (const std::exception& e)
    {
        if (quiet)
            fb::logger::warn("matchmaking confirm failed for character {}: {}", character_id, e.what());
        else
            throw;
    }

    if (!quiet)
    {
        auto match_type = this->_enrollment.has_value() ? this->_enrollment->match_type : 0;
        auto ptr        = this->owner.weak_from_this_as<character>().lock();
        if (ptr != nullptr)
        {
            auto match_id_str = std::string(match_id);
            auto lua = this->owner.server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_confirm");
            if (lua)
            {
                lua->pushobject(ptr);
                lua->pushstring(match_id_str.c_str());
                lua->pushinteger(match_type);
                std::ignore = lua->call(3);
            }
        }
    }

    co_return;
}

async::task<void> matchmaker::decline_queue(std::string_view match_id, bool quiet)
{
    this->owner.assert_thread();

    if (match_id.empty())
        co_return;

    auto world        = fb::config<uint32_t>("world");
    auto character_id = this->owner.id;

    try
    {
        auto&& resp = co_await this->owner.server.http.post("matchmaking",
                                                            "/matchmaking/decline",
                                                            mp_reqs::Decline{match_id, world, character_id});

        if (resp.error != 0)
        {
            if (quiet)
            {
                fb::logger::warn("matchmaking decline failed for character {}: {}",
                                 character_id,
                                 enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
            else
            {
                throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));
            }
        }
    }
    catch (const std::exception& e)
    {
        if (quiet)
            fb::logger::warn("matchmaking decline failed for character {}: {}", character_id, e.what());
        else
            throw;
    }

    this->clear_pending_match_id_if(match_id);

    if (!quiet)
    {
        auto match_type = this->_enrollment.has_value() ? this->_enrollment->match_type : 0;
        auto ptr        = this->owner.weak_from_this_as<character>().lock();
        if (ptr != nullptr)
        {
            auto match_id_str = std::string(match_id);
            auto lua = this->owner.server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_decline");
            if (lua)
            {
                lua->pushobject(ptr);
                lua->pushstring(match_id_str.c_str());
                lua->pushinteger(match_type);
                std::ignore = lua->call(3);
            }
        }
    }

    if (this->_enrollment.has_value())
    {
        auto registry_id_str = this->_enrollment->registry_id;
        auto match_type      = this->_enrollment->match_type;
        auto owner_id        = this->owner.id;
        auto guard           = this->owner.server.characters.enter_read();
        guard.value().foreach_enqueue(
            [registry_id_str, match_type](auto& ch) -> async::task<void> {
                auto lua = ch->server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_unregister");
                if (lua)
                {
                    lua->pushobject(ch);
                    lua->pushinteger(match_type);
                    lua->pushstring(registry_id_str.c_str());
                    std::ignore = lua->call(3);
                }
                ch->matchmaker.clear_enrollment();
                co_return;
            },
            [registry_id_str, owner_id](const character::container::character_ptr_t& ch) {
                if (ch->id == owner_id)
                    return false;

                auto id = ch->matchmaker.registry_id();
                return id.has_value() && id.value() == registry_id_str;
            });
    }

    this->clear_enrollment();
    co_return;
}

async::task<void> matchmaker::register_queue(uint32_t match_type)
{
    this->owner.assert_thread();

    auto weak = this->owner.weak_from_this_as<character>();
    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    auto self = weak.lock();
    if (self == nullptr)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    auto match_type_enum = static_cast<fb::model::enum_value::MATCH_TYPE>(match_type);
    if (table::matchmaking.contains(match_type_enum) == false)
        throw std::runtime_error(enum_tostring(fb::model::enum_value::ERROR_CODE::MATCHMAKING_UNKNOWN_QUEUE));

    auto& matchmaking_config = table::matchmaking[match_type_enum];
    auto  world              = fb::config<uint32_t>("world");
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

        auto  guard      = this->owner.server.characters.enter_read();
        auto& characters = guard.value();
        for (auto& name : member_names)
        {
            auto ch = characters.find(name);
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

            auto skill = ptr->matchmaker.ensure_skill(match_type);
            co_return mp::RegistryEntry{world, ptr->id, skill.mu, skill.sigma};
        };
        entries.push_back(co_await builder.dispatch());
    }

    if (entries.size() > matchmaking_config.member_count)
        throw std::runtime_error(enum_tostring(fb::model::enum_value::ERROR_CODE::UNHANDLED));

    auto&& resp = co_await this->owner.server.http.post("matchmaking",
                                                        "/matchmaking/register",
                                                        mp_reqs::Register{match_type, entries});
    if (resp.error != 0)
        throw std::runtime_error(enum_tostring(static_cast<fb::model::enum_value::ERROR_CODE>(resp.error)));

    this->set_enrollment(match_type, resp.registry_id);

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

                ptr->matchmaker.set_enrollment(match_type, registry_id_str);

                auto lua = ptr->server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_register");
                if (lua)
                {
                    lua->pushobject(ptr);
                    lua->pushinteger(match_type);
                    lua->pushstring(registry_id_str.c_str());
                    std::ignore = lua->call(3);
                }
                co_return;
            };
            builder.enqueue();
        }
    }

    {
        auto lua = this->owner.server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_register");
        if (lua)
        {
            lua->pushobject(self);
            lua->pushinteger(match_type);
            lua->pushstring(resp.registry_id.c_str());
            std::ignore = lua->call(3);
        }
    }

    co_return;
}
