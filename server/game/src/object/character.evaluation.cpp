#include <fb/game/character.h>

#include <fb/encoding.h>
#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <macro.h>

#include <json/json.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <format>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

using namespace fb::game;

character::evaluation::evaluation(character&                                     owner,
                                  const fb::model::datetime&                     now,
                                  uint16_t                                       score,
                                  uint32_t                                       playtime,
                                  const std::map<uint32_t, fb::model::datetime>& targets) :
    _owner(owner),
    _score(score),
    _tick(now),
    _playtime(std::min<std::chrono::milliseconds>(
        std::chrono::seconds(playtime),
        std::chrono::milliseconds(fb::model::const_value::reputation::EVALUATION_INTERVAL.total_milliseconds())))
{
    for (auto& [target, evaluated_at] : targets)
    {
        auto lowered = std::min(evaluated_at, now);
        if (cooling_down(lowered, now))
            this->_targets.insert_or_assign(target, lowered);
    }
}

bool character::evaluation::cooling_down(const fb::model::datetime& evaluated_at, const fb::model::datetime& now)
{
    return now - evaluated_at < fb::model::const_value::reputation::EVALUATION_COOLDOWN;
}

std::chrono::milliseconds character::evaluation::earned(const fb::model::datetime& now) const
{
    if (this->_owner.client_version != fb::protocol::CLIENT_VERSION::v651 ||
        this->_score >= fb::model::const_value::reputation::MAX_EVALUATION)
        return std::chrono::milliseconds(0);

    return std::max(std::chrono::milliseconds(0), std::chrono::milliseconds((now - this->_tick).total_milliseconds()));
}

void character::evaluation::assign(uint16_t value)
{
    this->_score = value;
    if (this->_score >= fb::model::const_value::reputation::MAX_EVALUATION)
        this->_playtime = std::chrono::milliseconds(0);
}

void character::evaluation::spend(uint32_t target, const fb::model::datetime& now)
{
    this->_score--;
    this->_targets.insert_or_assign(target, now);
}

void character::evaluation::refund(uint32_t target, const std::optional<fb::model::datetime>& previous)
{
    this->assign(std::min(static_cast<uint16_t>(this->_score + 1), fb::model::const_value::reputation::MAX_EVALUATION));
    if (previous.has_value())
        this->_targets.insert_or_assign(target, previous.value());
    else
        this->_targets.erase(target);
}

uint16_t character::evaluation::score() const
{
    this->_owner.assert_thread();
    return this->_score;
}

void character::evaluation::score(uint16_t value)
{
    this->_owner.assert_thread();
    this->assign(value);
    this->_owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character::evaluation::accumulate(const fb::model::datetime& now)
{
    this->_owner.assert_thread();

    auto earned      = this->earned(now);
    this->_tick      = now;
    this->_playtime += earned;

    auto interval =
        std::chrono::milliseconds(fb::model::const_value::reputation::EVALUATION_INTERVAL.total_milliseconds());
    if (this->_playtime < interval)
        return;

    this->_playtime -= interval;
    this->assign(this->_score + 1);
    this->_owner.message(_TEXT(MESSAGE_REPUTATION_EVALUATION_GAINED), MESSAGE_TYPE::STATE);
    this->_owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character::evaluation::shift(const fb::model::timespan& delta)
{
    this->_owner.assert_thread();

    this->_tick += delta;
    for (auto& [target, evaluated_at] : this->_targets)
    {
        evaluated_at += delta;
    }
}

async::task<void> character::evaluation::evaluate(std::string target_name, bool raise)
{
    this->_owner.assert_thread();

    if (this->_score == 0)
        throw std::runtime_error(_TEXT(MESSAGE_REPUTATION_NO_EVALUATION));

    auto target = this->_owner.server.characters.find(target_name);
    if (target == nullptr)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), target_name));

    if (target->id == this->_owner.id)
        throw std::runtime_error(_TEXT(MESSAGE_REPUTATION_SELF));

    auto now         = this->_owner.server.now();
    auto target_id   = target->id;
    auto role        = this->_owner.role();
    auto previous    = std::optional<fb::model::datetime>();
    auto on_cooldown = false;
    auto found       = this->_targets.find(target_id);
    if (found != this->_targets.end())
    {
        on_cooldown = cooling_down(found->second, now);
        previous    = found->second;
    }

    // Spend the ticket before leaving this thread so a second request cannot reuse it.
    if (on_cooldown == false)
        this->spend(target_id, now);

    auto& server      = this->_owner.server;
    auto  user_id     = this->_owner.id;
    auto  user_name   = this->_owner.name();
    auto  self_weak   = this->_owner.weak_from_this_as<character>();
    auto  target_weak = target->weak_from_this_as<character>();
    target            = nullptr;

    // The target is read only on its own thread, and a hidden target answers like a logged out one even on cooldown.
    auto visible           = false;
    auto target_reputation = std::optional<int16_t>();
    try
    {
        co_await server.threads.switching(target_weak);
        target = target_weak.lock();
        if (target != nullptr && target->inited() && target->hidden(role) == false)
        {
            visible     = true;
            target_name = target->name();
            if (on_cooldown == false)
            {
                auto value = std::clamp<int32_t>(target->reputation() + (raise ? 1 : -1),
                                                 std::numeric_limits<int16_t>::min(),
                                                 std::numeric_limits<int16_t>::max());
                target->reputation(static_cast<int16_t>(value));
                target_reputation = target->reputation();
                target->message(raise ? _TEXT(MESSAGE_REPUTATION_RAISED_BY_OTHER)
                                      : _TEXT(MESSAGE_REPUTATION_LOWERED_BY_OTHER),
                                MESSAGE_TYPE::STATE);

                auto log_data           = Json::Value();
                log_data["user_id"]     = static_cast<Json::Int64>(user_id);
                log_data["user_name"]   = UTF8(user_name, PLATFORM::WINDOWS);
                log_data["target_id"]   = static_cast<Json::Int64>(target_id);
                log_data["target_name"] = UTF8(target_name, PLATFORM::WINDOWS);
                log_data["raise"]       = raise;
                log_data["reputation"]  = static_cast<Json::Int>(target_reputation.value());
                server.log.write("reputation", log_data);
            }
        }
    }
    catch (std::exception&)
    { }
    target = nullptr;

    try
    {
        co_await server.threads.switching(self_weak);
    }
    catch (std::exception&)
    {
        co_return;
    }

    auto self = self_weak.lock();
    if (self == nullptr)
        co_return;

    if (target_reputation.has_value())
    {
        self->message(raise ? std::format(_TEXT(MESSAGE_REPUTATION_RAISED), target_name)
                            : std::format(_TEXT(MESSAGE_REPUTATION_LOWERED), target_name),
                      MESSAGE_TYPE::STATE);
        self->update(UPDATE_STATE_LEVEL::BASED);
    }
    else
    {
        if (on_cooldown == false)
        {
            self->evaluation.refund(target_id, previous);
            self->update(UPDATE_STATE_LEVEL::BASED);
        }

        if (visible)
            throw std::runtime_error(_TEXT(MESSAGE_REPUTATION_ALREADY_EVALUATED));
        else
            throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), target_name));
    }
}

void character::evaluation::to_protocol(fb::protocol::internal::Character& dto) const
{
    auto now                = this->_owner.server.now();
    dto.evaluation          = this->_score;
    dto.evaluation_playtime = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(this->_playtime + this->earned(now)).count());
    for (auto& [target, evaluated_at] : this->_targets)
    {
        if (cooling_down(evaluated_at, now))
            dto.evaluated_targets.push_back(fb::protocol::internal::EvaluatedTarget(target, evaluated_at.to_string()));
    }
}
