#include <fb/game/server.h>

#include <fb/amqp_route.h>
#include <fb/game/handler.h>
#include <fb/log_collector.h>
#include <fb/logger.h>

#include <json/json.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;

namespace game_resp     = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

uint8_t fb::game::server::id() const
{
    return fb::config<uint8_t>("id");
}

internal::Service fb::game::server::service() const
{
    return internal::Service::Game;
}

void fb::game::server::send(object&                     object,
                            const fb::protocol::header& header,
                            fb::game::scope             scope,
                            send_option                 options)
{
    auto weak   = object.weak_from_this_as<fb::game::object>();
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    header.serialize(writer);

    auto shared_ptr = weak.lock();
    if (shared_ptr == nullptr)
        return;

    auto allowed = [&options](fb::game::object& to) -> bool {
        if (!options.condition)
            return true;

        return options.condition(to);
    };

    switch (scope)
    {
    case fb::game::scope::PIVOT:
    {
        if (options.with_me)
            shared_ptr->send(stream, options.encrypt);

        for (auto& x : shared_ptr->nears(OBJECT_TYPE::CHARACTER, true))
        {
            if (x->sight(*shared_ptr) == false)
                continue;

            if (shared_ptr->hidden(*x))
                continue;

            if (allowed(*x) == false)
                continue;

            x->send(stream, options.encrypt);
        }
    }
    break;

    case fb::game::scope::GROUP:
    {
        if (shared_ptr->is(OBJECT_TYPE::CHARACTER) == false)
            return;

        auto& ch       = static_cast<const character&>(*shared_ptr);
        auto& group_id = ch.group_id();
        if (group_id.has_value() == false)
            return;

        {
            auto  guard = this->groups.enter_read(group_id.value());
            auto& group = guard.value();
            for (auto& member : group->characters())
            {
                if (member == nullptr)
                    continue;

                if (allowed(*member) == false)
                    continue;

                member->send(stream, options.encrypt);
            }
        }
    }
    break;

    case fb::game::scope::MAP:
    {
        auto map = shared_ptr->map();
        if (map == nullptr)
            return;

        for (const auto& [seq, obj] : map->objects)
        {
            if (obj->oid() == shared_ptr->oid())
            {
                if (options.with_me == false)
                    continue;
            }
            else if (allowed(*obj) == false)
            {
                continue;
            }

            obj->send(stream, options.encrypt);
        }
    }
    break;

    case fb::game::scope::WORLD:
    {
        this->characters.send(stream, options.encrypt);
    }
    break;
    }
}

void fb::game::server::sync_time()
{
    auto updated = this->now();
    if (this->_time.hours() != updated.hours() || this->_time.minutes() != updated.minutes())
    {
        this->characters.update_time(static_cast<uint8_t>(updated.hours()), static_cast<uint8_t>(updated.minutes()));
    }

    this->_time = updated;
    this->weather.sync();
}

uint8_t fb::game::server::brightness_from_time(uint8_t hours, uint8_t minutes)
{
    if (hours >= 1)
        return 20;

    const auto t = static_cast<uint32_t>(minutes) * 60u;
    double     v;
    if (t <= 1200)
        v = 1.0 - (static_cast<double>(450 * t / 3600) * 0.0043333336);
    else if (t <= 2400)
        v = 0.35;
    else if (t < 3600)
        v = (static_cast<double>(450 * t / 3600) - 300.0) * 0.0043333336 + 0.35;
    else
        v = 1.0;

    auto value = static_cast<int>(std::lround((v - 0.35) / 0.65 * 20.0));
    if (value < 0)
        value = 0;
    if (value > 20)
        value = 20;
    return static_cast<uint8_t>(value);
}

uint8_t fb::game::server::brightness() const
{
    return brightness_from_time(static_cast<uint8_t>(this->_time.hours()), static_cast<uint8_t>(this->_time.minutes()));
}

async::task<bool> fb::game::server::save(character& ch)
{
    // Save overwrites items, spells, achievements and quests; a partially loaded character would wipe them.
    if (ch.inited() == false || ch.loaded() == false)
        co_return false;

    if (!fb::config<std::optional<uint32_t>>("world") && ch.has_return_point() == false)
    {
        fb::logger::fatal("Character {} cross save without home snapshot", ch.name());
        co_return false;
    }

    auto weak      = ch.weak_from_this_as<character>();
    auto save_lock = ch.save_lock;
    co_await save_lock->lock();

    auto success = false;
    try
    {
        co_await this->threads.switching(weak);
        auto locked = weak.lock();
        if (locked == nullptr)
            throw std::runtime_error("character destroyed before its save snapshot");

        auto world   = locked->world();
        auto payload = this->save_payload(*locked);
        locked.reset();

        auto&& resp = co_await this->http.post("internal",
                                               "/in-game/save",
                                               internal_reqs::Save{world, payload, fb::config<uint8_t>("id")});
        success     = resp.success;
    }
    catch (...)
    {
        save_lock->unlock();
        throw;
    }
    save_lock->unlock();

    co_await this->threads.switching(weak);
    auto shared = weak.lock();
    if (shared == nullptr)
        co_return success;

    shared->save_ack();
    co_return success;
}

async::task<internal_resp::Ban> fb::game::server::ban(std::string_view               actor,
                                                      std::string_view               name,
                                                      std::string_view               reason,
                                                      const std::optional<uint32_t>& days)
{
    auto name_str = std::string(name);
    auto target   = this->characters.find(name_str);
    auto world    = uint32_t{0};
    if (target != nullptr)
    {
        world = target->world();
    }
    else if (auto process_world = fb::config<std::optional<uint32_t>>("world"))
    {
        world = *process_world;
    }
    else
    {
        throw std::runtime_error("교차 서버에서는 접속 중인 플레이어만 제재할 수 있습니다.");
    }

    auto   reason_str = std::string(reason);
    auto   actor_str  = std::string(actor);
    auto&& resp       = co_await this->http.post("internal",
                                           "/ban/add",
                                           internal_reqs::Ban{world, name_str, reason_str, days, actor_str});
    co_return std::move(resp);
}

async::task<internal_resp::Unban> fb::game::server::unban(std::string_view actor, std::string_view name)
{
    auto name_str = std::string(name);
    auto target   = this->characters.find(name_str);
    auto world    = uint32_t{0};
    if (target != nullptr)
    {
        world = target->world();
    }
    else if (auto process_world = fb::config<std::optional<uint32_t>>("world"))
    {
        world = *process_world;
    }
    else
    {
        throw std::runtime_error("교차 서버에서는 접속 중인 플레이어만 제재할 수 있습니다.");
    }

    auto   actor_str = std::string(actor);
    auto&& resp = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{world, name_str, actor_str});
    co_return std::move(resp);
}

internal::SavePayload fb::game::server::save_payload(const character& ch) const
{
    auto items = std::vector<internal::Item>();
    for (auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = ch.items[i];
        if (item == nullptr)
            continue;

        auto protocol  = item->to_protocol();
        protocol.index = i;
        items.push_back(protocol);
    }

    for (auto i : ch.items.escrow_indices())
    {
        auto escrow           = ch.items.escrow(i);
        auto protocol         = escrow->item->to_protocol();
        protocol.index        = i;
        protocol.stored       = ESCROW_STORED;
        protocol.listing_id   = escrow->listing_id;
        protocol.locked_money = escrow->money;
        items.push_back(protocol);
    }

    for (auto& [parts, equipment] : ch.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        items.push_back(equipment->to_protocol(parts));
    }

    auto& stored_items = ch.items.stored();
    for (int i = 0; i < stored_items.size(); i++)
    {
        auto item       = stored_items.at(i);
        auto protocol   = item->to_protocol();
        protocol.stored = i;
        items.push_back(protocol);
    }

    auto spells = std::vector<internal::Spell>();
    for (uint8_t i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = ch.spells[i];
        if (spell == nullptr)
            continue;

        spells.push_back(internal::Spell{ch.id, i, spell->model().id, spell->next().to_string()});
    }

    auto achievements = std::vector<internal::Achievement>();
    for (auto& [id, achievement] : ch.achievements)
    {
        achievements.push_back(
            internal::Achievement{ch.id, id, achievement->text, achievement->icon, achievement->color});
    }

    auto quests = std::vector<internal::Quest>();
    for (auto& [qid, quest] : ch.quests)
    {
        quests.push_back(
            internal::Quest{ch.id, qid, quest->step(), quest->progress(), quest->param(), quest->completed()});
    }

    auto marketplace_pendings = ch.marketplace.to_save_dtos();
    auto matchmaking_skills   = ch.matchmaker.to_protocol();
    auto collection_unlocks   = ch.collections.to_protocol(ch.id);
    auto snapshot_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    return internal::SavePayload(ch.to_protocol(),
                                 ch.marriage().to_protocol(),
                                 items,
                                 spells,
                                 matchmaking_skills,
                                 achievements,
                                 quests,
                                 marketplace_pendings,
                                 collection_unlocks,
                                 snapshot_time);
}

async::task<void> fb::game::server::save(fb::thread& thread)
{
    static constexpr size_t SAVE_BATCH_CHUNK_SIZE = 100;

    auto params   = thread.template data<thread_params>();
    auto by_world = std::map<uint32_t, std::vector<std::weak_ptr<character>>>{};
    co_await params->characters.foreach ([&](auto& character) {
        if (!character->inited() || !character->loaded())
            return;

        if (!fb::config<std::optional<uint32_t>>("world") && character->has_return_point() == false)
        {
            fb::logger::fatal("Character {} cross save without home snapshot", character->name());
            return;
        }

        by_world[character->world()].push_back(character);
    });

    for (auto& [world, characters] : by_world)
    {
        for (size_t offset = 0; offset < characters.size(); offset += SAVE_BATCH_CHUNK_SIZE)
        {
            const size_t chunk_end = (std::min)(offset + SAVE_BATCH_CHUNK_SIZE, characters.size());

            auto saving = std::vector<std::pair<std::weak_ptr<character>, std::shared_ptr<fb::async_shared_mutex>>>{};
            auto batch  = std::vector<internal::SavePayload>{};
            for (size_t i = offset; i < chunk_end; i++)
            {
                auto shared = characters[i].lock();
                if (shared == nullptr || shared->matched_thread() == false)
                    continue;

                if (shared->save_lock->try_lock() == false)
                    continue;

                saving.push_back({characters[i], shared->save_lock});
                batch.push_back(this->save_payload(*shared));
            }
            if (batch.empty())
                continue;

            auto count  = batch.size();
            auto failed = false;
            try
            {
                std::ignore = co_await this->http.post(
                    "internal",
                    "/in-game/save-batch",
                    internal_reqs::SaveBatch{world, std::move(batch), fb::config<uint8_t>("id")});
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("save batch failed: world={} count={} error={}", world, count, e.what());
                failed = true;
            }

            for (auto& entry : saving)
                entry.second->unlock();

            if (failed)
            {
                co_await thread.switching();
                continue;
            }

            for (auto& entry : saving)
            {
                auto weak   = entry.first;
                auto shared = weak.lock();
                if (shared == nullptr)
                    continue;

                // The character may have moved to another thread while the save request was in flight.
                if (shared->matched_thread())
                {
                    shared->save_ack();
                }
                else
                {
                    auto builder = this->threads.new_builder<void, character>(weak);
                    builder.func = [weak](auto&) -> async::task<void> {
                        auto moved = weak.lock();
                        if (moved != nullptr)
                            moved->save_ack();
                        co_return;
                    };
                    builder.on_error = [](std::exception& e) {
                        fb::logger::fatal("save_ack error: {}", e.what());
                    };
                    builder.enqueue();
                }
            }
        }
    }
}

async::task<void> fb::game::server::save()
{
    auto tasks = std::vector<async::task<void>>{};
    tasks.reserve(this->threads.count());
    for (auto& [id, thread] : this->threads)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [this](auto& thread) -> async::task<void> {
            co_await this->save(thread);
        };
        tasks.push_back(builder.dispatch());
    }
    for (auto& t : tasks)
    {
        co_await t;
    }
}
const fb::model::datetime& fb::game::server::time() const
{
    return this->_time;
}
async::task<void> fb::game::server::update_status()
{
    try
    {
        auto world = fb::config<std::optional<uint32_t>>("world");
        std::ignore =
            co_await this->http.post("internal",
                                     "/server/heartbeat",
                                     internal_reqs::Heartbeat{world,
                                                              internal::Service::Game,
                                                              this->id(),
                                                              this->name(),
                                                              fb::config<std::string_view>("ip"),
                                                              fb::config<uint16_t>("port"),
                                                              static_cast<uint32_t>(this->characters.size())});
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to send heartbeat: {}", e.what());
    }
}

double fb::game::server::exp_multiplier() const
{
    return this->_exp_multiplier;
}

void fb::game::server::exp_multiplier(double value)
{
    this->_exp_multiplier = value;
}

double fb::game::server::drop_rate_multiplier() const
{
    return this->_drop_rate_multiplier;
}

void fb::game::server::drop_rate_multiplier(double value)
{
    this->_drop_rate_multiplier = value;
}
