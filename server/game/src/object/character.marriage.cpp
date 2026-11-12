#include <fb/game/character.h>

#include <fb/game/server.h>
#include <fb/logger.h>
#include <macro.h>

#include <array>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>

using namespace fb::game;

namespace internal_reqs = fb::protocol::internal::request;

character::marriage::marriage(character& owner) :
    _owner(owner),
    _remarriage_after(owner.server.now())
{ }

const std::optional<uint32_t>& character::marriage::spouse_id() const
{
    this->_owner.assert_thread();
    return this->_spouse_id;
}

const std::string& character::marriage::spouse_name() const
{
    this->_owner.assert_thread();
    return this->_spouse_name;
}

uint32_t character::marriage::divorce_count() const
{
    this->_owner.assert_thread();
    return this->_divorce_count;
}

bool character::marriage::remarryable() const
{
    this->_owner.assert_thread();
    return this->_spouse_id.has_value() == false && this->_owner.server.now() >= this->_remarriage_after;
}

std::optional<std::string> character::marriage::refusal(const character& target) const
{
    this->_owner.assert_thread();

    auto min_level = static_cast<uint32_t>(fb::model::const_value::marriage::MIN_LEVEL);
    auto map       = this->_owner.map();
    if (this->_spouse_id.has_value())
        return "이미 결혼을 하셨습니다.";

    if (this->remarryable() == false)
        return "재혼 대기 시간이 지나지 않았습니다. 잠시 후에 다시 시도해 주세요.";

    if (this->_owner.level() < min_level)
        return std::format("레벨 {} 이상만 결혼할 수 있습니다.", min_level);

    if (&target == &this->_owner)
        return "자신과는 결혼할 수 없습니다.";

    if (this->_owner.thread() != target.thread() || map == nullptr || map != target.map())
        return "상대와 같은 곳에 있어야 결혼할 수 있습니다.";

    if (this->_owner.world() != target.world())
        return "다른 서버 출신과는 결혼할 수 없습니다.";

    if (this->_owner.gender() == target.gender())
        return "같은 성별끼리는 결혼할 수 없습니다.";

    if (target.marriage._spouse_id.has_value())
        return "이미 결혼한 상대와는 결혼을 할 수 없습니다.";

    if (target.marriage.remarryable() == false)
        return "상대방은 아직 재혼할 수 없습니다.";

    if (target.level() < min_level)
        return std::format("결혼할 상대의 레벨이 {} 이상이어야 합니다.", min_level);

    return std::nullopt;
}

async::task<std::optional<std::string>> character::marriage::marry(std::shared_ptr<character> target)
{
    this->_owner.assert_thread();

    auto  me          = this->_owner.shared_from_this_as<character>();
    auto  weak        = std::weak_ptr<character>(me);
    auto  target_weak = std::weak_ptr<character>(target);
    auto& server      = me->server;
    auto  locks       = co_await this->lock(*target);
    auto  error       = std::optional<std::string>{};
    try
    {
        co_await server.threads.switching(weak);
        error = this->refusal(*target);
        if (error.has_value() == false)
        {
            auto my_after     = this->_remarriage_after;
            auto target_after = target->marriage._remarriage_after;
            auto now          = server.now();

            this->_spouse_id                   = target->id;
            this->_spouse_name                 = target->name();
            this->_remarriage_after            = now;
            target->marriage._spouse_id        = me->id;
            target->marriage._spouse_name      = me->name();
            target->marriage._remarriage_after = now;
            me->update_internal();
            target->update_internal();

            auto refused = std::optional<uint32_t>{};
            try
            {
                auto resp =
                    co_await server.http.post(weak,
                                              "internal",
                                              "/marriage/marry",
                                              internal_reqs::Marry{me->world(), me->id, target->id, now.to_string()});
                if (resp.error != static_cast<uint32_t>(ERROR_CODE::NONE))
                    refused = resp.error;
            }
            catch (fb::http_error& e)
            {
                fb::logger::warn("marry request failed: {}", e.what());
                error = _TEXT(MESSAGE_REQUEST_UNCONFIRMED);
            }

            if (refused.has_value())
            {
                this->_spouse_id = std::nullopt;
                this->_spouse_name.clear();
                this->_remarriage_after = my_after;
                me->update_internal();

                co_await server.threads.switching(target_weak);
                target->marriage._spouse_id = std::nullopt;
                target->marriage._spouse_name.clear();
                target->marriage._remarriage_after = target_after;
                target->update_internal();
                co_await server.threads.switching(weak);

                error = std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), refused.value());
            }
        }
    }
    catch (...)
    {
        for (auto& x : locks)
        {
            x->unlock();
        }

        throw;
    }

    for (auto& x : locks)
    {
        x->unlock();
    }

    co_return error;
}

async::task<std::optional<std::string>> character::marriage::divorce()
{
    this->_owner.assert_thread();

    if (this->_spouse_id.has_value() == false)
        co_return "결혼을 한 사람만이 이혼을 할 수 있습니다.";

    auto& server = this->_owner.server;
    auto  spouse = server.characters.find(this->_spouse_id.value());
    if (spouse == nullptr || spouse->hidden(this->_owner))
        co_return "배우자가 현재 접속 중이 아닙니다.";

    auto me          = this->_owner.shared_from_this_as<character>();
    auto weak        = std::weak_ptr<character>(me);
    auto spouse_weak = std::weak_ptr<character>(spouse);
    auto locks       = co_await this->lock(*spouse);
    auto error       = std::optional<std::string>{};
    try
    {
        co_await server.threads.switching(weak);

        auto map = me->map();
        if (this->_spouse_id != spouse->id)
        {
            error = "결혼을 한 사람만이 이혼을 할 수 있습니다.";
        }
        else if (spouse->thread() != me->thread() || map == nullptr || map != spouse->map())
        {
            error = "배우자와 같은 곳에 있어야 이혼할 수 있습니다.";
        }
        else
        {
            auto my_after     = this->_remarriage_after;
            auto spouse_after = spouse->marriage._remarriage_after;
            auto after        = server.now();
            after.add_days(7);

            for (auto* x : {&this->_owner, spouse.get()})
            {
                x->marriage._spouse_id = std::nullopt;
                x->marriage._spouse_name.clear();
                x->marriage._remarriage_after = after;
                x->marriage._divorce_count++;
                x->update_internal();
            }

            auto refused = std::optional<uint32_t>{};
            try
            {
                auto resp = co_await server.http.post(
                    weak,
                    "internal",
                    "/marriage/divorce",
                    internal_reqs::Divorce{me->world(), me->id, spouse->id, after.to_string()});
                if (resp.error != static_cast<uint32_t>(ERROR_CODE::NONE))
                    refused = resp.error;
            }
            catch (fb::http_error& e)
            {
                fb::logger::warn("divorce request failed: {}", e.what());
                error = _TEXT(MESSAGE_REQUEST_UNCONFIRMED);
            }

            if (refused.has_value())
            {
                this->_spouse_id        = spouse->id;
                this->_spouse_name      = spouse->name();
                this->_remarriage_after = my_after;
                this->_divorce_count--;
                me->update_internal();

                co_await server.threads.switching(spouse_weak);
                spouse->marriage._spouse_id        = me->id;
                spouse->marriage._spouse_name      = me->name();
                spouse->marriage._remarriage_after = spouse_after;
                spouse->marriage._divorce_count--;
                spouse->update_internal();
                co_await server.threads.switching(weak);

                error = std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), refused.value());
            }
        }
    }
    catch (...)
    {
        for (auto& x : locks)
        {
            x->unlock();
        }

        throw;
    }

    for (auto& x : locks)
    {
        x->unlock();
    }

    co_return error;
}

void character::marriage::restore(const fb::protocol::internal::Marriage& row)
{
    this->_owner.assert_thread();

    this->_spouse_id   = row.spouse_id;
    this->_spouse_name = row.spouse_name;
    this->_remarriage_after =
        row.remarriage_after.empty() ? this->_owner.server.now() : fb::model::datetime(row.remarriage_after);
    this->_divorce_count = row.divorce_count;
    this->_owner.update_internal();
}

async::task<std::array<std::shared_ptr<fb::async_shared_mutex>, 2>> character::marriage::lock(character& spouse)
{
    auto locks = this->_owner.id < spouse.id ? std::array{this->_write_lock, spouse.marriage._write_lock}
                                             : std::array{spouse.marriage._write_lock, this->_write_lock};
    for (auto& x : locks)
    {
        co_await x->lock();
    }

    co_return locks;
}
