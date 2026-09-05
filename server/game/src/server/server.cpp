#include <fb/game/server.h>
#include <fb/game/handler.h>
#include <fb/log_collector.h>
#include <fb/logger.h>
#include <fb/amqp_route.h>
#include <json/json.h>
#include <cmath>
#include <tuple>
#include <map>

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

async::task<void> fb::game::server::save(character& ch)
{
    if (ch.inited() == false)
        co_return;
    if (fb::is_cross() && ch.has_return_point() == false)
    {
        fb::logger::fatal("Character {} cross save without home snapshot", ch.name());
        co_return;
    }

    auto weak    = ch.weak_from_this();
    auto world   = ch.world();
    auto payload = this->save_payload(ch);
    std::ignore  = co_await this->http.post("internal", "/in-game/save", internal_reqs::Save{world, payload});

    co_await this->threads.switching(weak);
    ch.save_ack();
}

async::task<internal_resp::Ban> fb::game::server::ban(std::string_view               name,
                                                      std::string_view               reason,
                                                      const std::optional<uint32_t>& days)
{
    auto   name_str   = std::string(name);
    auto   reason_str = std::string(reason);
    auto   world      = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->http.post("internal", "/ban/add", internal_reqs::Ban{world, name_str, reason_str, days});
    co_return std::move(resp);
}

async::task<internal_resp::Unban> fb::game::server::unban(std::string_view name)
{
    auto   name_str = std::string(name);
    auto   world    = fb::config<uint32_t>("world");
    auto&& resp     = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{world, name_str});
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

    return internal::SavePayload(ch.to_protocol(),
                                 ch.marriage().to_protocol(),
                                 items,
                                 spells,
                                 matchmaking_skills,
                                 achievements,
                                 quests,
                                 marketplace_pendings,
                                 collection_unlocks);
}

async::task<void> fb::game::server::save()
{
    static constexpr size_t SAVE_BATCH_CHUNK_SIZE = 100;
    auto                    tasks                 = std::vector<async::task<void>>{};
    tasks.reserve(this->threads.count());
    for (auto& [id, thread] : this->threads)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [this](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            auto by_world =
                std::map<uint32_t, std::pair<std::vector<character*>, std::vector<internal::SavePayload>>>{};
            co_await params->characters.foreach ([&](auto& character) {
                if (!character->inited())
                    return;
                if (fb::is_cross() && character->has_return_point() == false)
                {
                    fb::logger::fatal("Character {} cross save without home snapshot", character->name());
                    return;
                }

                auto& chunk = by_world[character->world()];
                chunk.first.push_back(character.get());
                chunk.second.push_back(this->save_payload(*character));
            });

            for (auto& [world, chunk] : by_world)
            {
                auto&        characters = chunk.first;
                auto&        payloads   = chunk.second;
                const size_t total      = payloads.size();
                for (size_t offset = 0; offset < total; offset += SAVE_BATCH_CHUNK_SIZE)
                {
                    const size_t chunk_end = (std::min)(offset + SAVE_BATCH_CHUNK_SIZE, total);
                    auto         batch =
                        std::vector<internal::SavePayload>(payloads.begin() + static_cast<std::ptrdiff_t>(offset),
                                                           payloads.begin() + static_cast<std::ptrdiff_t>(chunk_end));
                    std::ignore = co_await this->http.post("internal",
                                                           "/in-game/save-batch",
                                                           internal_reqs::SaveBatch{world, std::move(batch)});

                    for (size_t i = offset; i < chunk_end; i++)
                    {
                        characters[i]->save_ack();
                    }
                }
            }
            co_return;
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
        auto world  = fb::config<uint32_t>("world");
        std::ignore = co_await this->http.post("internal",
                                               "/server/heartbeat",
                                               internal_reqs::Heartbeat{world,
                                                                        internal::Service::Game,
                                                                        this->id(),
                                                                        this->name(),
                                                                        fb::config<std::string_view>("ip"),
                                                                        fb::config<uint16_t>("port"),
                                                                        fb::process_role()});
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
