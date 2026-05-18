#include <fb/game/server.h>
#include <fb/game/handler.h>
#include <fb/game/builtin/server.h>
#include <fb/lua.h>
#include <fb/log_collector.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <json/writer.h>
#include <sstream>

using namespace fb::game;
using namespace std::chrono_literals;
using table = fb::model::table;

namespace game_resp     = fb::protocol::game::response;
namespace game_reqs     = fb::protocol::game::request;
namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<character>(io_context, "GAME", port, fb::config<uint32_t>("http:max_concurrent", 500)),
    maps(*this, fb::config<uint32_t>("id")),
    listener(*this),
    characters(*this),
    storage_pending(*this),
    clans([](const std::shared_ptr<clan>& clan) -> uint32_t {
        return clan->id();
    }),
    groups([](const std::shared_ptr<group>& group) -> uint32_t {
        return group->id();
    }),
    _map_update_cache(
        [](const map::cache_bytes& cache_bytes) -> uint64_t {
            return cache_bytes.hash;
        },
        1024),
    log(fb::config<std::string>("amqp:log:ip"),
        fb::config<uint16_t>("amqp:log:port"),
        fb::config<std::string>("amqp:log:uid"),
        fb::config<std::string>("amqp:log:pwd"),
        std::to_string(fb::config<uint32_t>("id")),
        fb::config<std::string>("name"),
        fb::config<uint32_t>("world")),
    _exp_multiplier(fb::config<double>("exp_multiplier")),
    _drop_rate_multiplier(fb::config<double>("drop_rate_multiplier"))
{
    auto& ist = fb::lua::context_pool::ist();
    ist.setup(this->threads);

    lua::env<fb::game::server>("server", this);
    lua::build<quest, lua::luable>();
    lua::build<door, lua::luable>();
    lua::build<clan, lua::luable>();
    lua::build<clan_member, lua::luable>();
    lua::build<achievement, lua::luable>();
    lua::build<spell, lua::luable>();
    lua::build<buff, lua::luable>();
    lua::build<map, fb::thread_switchable>();
    lua::build<group, fb::thread_switchable>();
    lua::build<object, fb::thread_switchable>();
    lua::build<life, object>();
    lua::build<mob, life>();
    lua::build<npc, object>();
    lua::build<item, object>();
    lua::build<equipment, item>();
    lua::build<weapon, equipment>();
    lua::build<character, life>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::object, lua::luable>();
    lua::build<fb::model::life, fb::model::object>();
    lua::build<fb::model::mob, fb::model::life>();
    lua::build<fb::model::npc, fb::model::object>();
    lua::build<fb::model::item, fb::model::object>();
    lua::build<fb::model::equipment, fb::model::item>();
    lua::build<fb::model::weapon, fb::model::equipment>();

    lua::build("log", builtin::server::builtin_log);
    lua::build("seed", builtin::server::builtin_seed);
    lua::build("sleep", builtin::server::builtin_sleep);
    lua::build("now", builtin::server::builtin_now);
    lua::build("time_forward", builtin::server::builtin_time_forward);
    lua::build("time_backward", builtin::server::builtin_time_backward);
    lua::build("datetime", builtin::server::builtin_datetime);
    lua::build("name2mob", builtin::server::builtin_name2mob);
    lua::build("name2spell", builtin::server::builtin_name2spell);
    lua::build("name2item", builtin::server::builtin_name2item);
    lua::build("name2npc", builtin::server::builtin_name2npc);
    lua::build("name2map", builtin::server::builtin_name2map);
    lua::build("name2ch", builtin::server::builtin_name2ch);
    lua::build("id2mob", builtin::server::builtin_id2mob);
    lua::build("id2spell", builtin::server::builtin_id2spell);
    lua::build("id2item", builtin::server::builtin_id2item);
    lua::build("id2npc", builtin::server::builtin_id2npc);
    lua::build("id2map", builtin::server::builtin_id2map);
    lua::build("id2ch", builtin::server::builtin_id2ch);
    lua::build("broadcast", builtin::server::builtin_broadcast);
    lua::build("assert_alive", builtin::server::builtin_assert_alive);
    lua::build("pursuit_sell", builtin::server::builtin_pursuit_sell);
    lua::build("pursuit_sell_price", builtin::server::builtin_pursuit_sell_price);
    lua::build("pursuit_sell_name", builtin::server::builtin_pursuit_sell_name);
    lua::build("pursuit_buy", builtin::server::builtin_pursuit_buy);
    lua::build("timer", builtin::server::builtin_timer);
    lua::build("weather", builtin::server::builtin_weather);
    lua::build("bright", builtin::server::builtin_bright);
    lua::build("name_with", builtin::server::builtin_name_with);
    lua::build("assert_korean", builtin::server::builtin_assert_korean);
    lua::build("CP949", builtin::server::builtin_cp949);
    lua::build("debug", builtin::server::builtin_debug);
    lua::build("name2class", builtin::server::builtin_name2class);
    lua::build("class2name", builtin::server::builtin_class2name);
    lua::build("save", builtin::server::builtin_save);
    lua::build("mknpc", builtin::server::builtin_mknpc);
    lua::build("maps", builtin::server::builtin_maps);
    lua::build("shutdown", builtin::server::builtin_shutdown);
    lua::build("ban", builtin::server::builtin_ban);
    lua::build("unban", builtin::server::builtin_unban);
    lua::build("regex", builtin::server::builtin_regex);
    lua::build("exp_multiplier", builtin::server::builtin_exp_multiplier);
    lua::build("drop_rate_multiplier", builtin::server::builtin_drop_rate_multiplier);
    lua::build("gv", builtin::server::builtin_gv);

    for (auto& [_, root] : ist)
    {
        auto& thread = root->initial_thread();
        std::ignore  = thread.dispatch([&root](auto&) -> async::task<void> {
            fb::model::lua::map_enum(*root);
            fb::model::lua::map_const(*root);
            co_return;
        });
    }
}

server::~server()
{ }

async::task<void> server::on_start()
{
    // deletor no longer needed - RAII handles cleanup automatically

    co_await fb::acceptor<character>::on_start();

    auto maps_division = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::game::map>>>{};
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        maps_division.insert({thread, std::vector<std::shared_ptr<fb::game::map>>{}});
    }

    for (auto& [id, map] : this->maps)
    {
        auto thread = this->threads.modular(id);
        maps_division[thread].push_back(map);
    }

    auto async_tasks = std::vector<async::task<void>>();
    for (auto& [thread, maps] : maps_division)
    {
        async_tasks.push_back(thread->dispatch([this, maps = std::move(maps)](auto& thread) -> async::task<void> {
            auto params = std::make_unique<thread_params>(*this);
            for (const auto& map : maps)
            {
                params->maps.insert({map->model.id, map});
                if (table::mob_spawn.contains(map->model.id))
                {
                    for (auto& spawn : table::mob_spawn[map->model.id])
                    {
                        params->rezens.push_back(fb::game::rezen(*this, spawn));
                    }
                }
            }
            thread.data(std::move(params));
            co_return;
        }));
    }

    for (auto& async_task : async_tasks)
    {
        async::awaitable_get(async_task);
    }

    this->handler.protocol.bind<fb::game::handler::protocol::login>();           // Game server connection handler
    this->handler.protocol.bind<fb::game::handler::protocol::direction>();       // Direction change handler
    this->handler.protocol.bind<fb::game::handler::protocol::exit>();            // Disconnect handler
    this->handler.protocol.bind<fb::game::handler::protocol::update_move>();     // Update move and map data handler
    this->handler.protocol.bind<fb::game::handler::protocol::move>();            // Move handler
    this->handler.protocol.bind<fb::game::handler::protocol::move_blocked>();    // Move blocked
    this->handler.protocol.bind<fb::game::handler::protocol::attack>();          // Attack handler
    this->handler.protocol.bind<fb::game::handler::protocol::loot>();            // Loot handler
    this->handler.protocol.bind<fb::game::handler::protocol::emotion>();         // Emotion handler
    this->handler.protocol.bind<fb::game::handler::protocol::map_update>();      // Map data update handler
    this->handler.protocol.bind<fb::game::handler::protocol::screen_refresh>();  // Screen refresh handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_active>();     // Item use handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_inactive>();   // Item unequip handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_drop>();       // Item drop handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_drop_money>(); // Money drop handler
    this->handler.protocol.bind<fb::game::handler::protocol::front_info>();      // Front info handler
    this->handler.protocol.bind<fb::game::handler::protocol::self_info>();       // Self info handler
    this->handler.protocol.bind<fb::game::handler::protocol::update_option>();   // Option setting handler
    this->handler.protocol.bind<fb::game::handler::protocol::click>();           // Object click handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_info>();       // Inventory right-click handler
    this->handler.protocol.bind<fb::game::handler::protocol::item_combine>();    // Item combine handler
    this->handler.protocol.bind<fb::game::handler::protocol::trade>();           // Trade handler
    this->handler.protocol.bind<fb::game::handler::protocol::group>();           // Group handler
    this->handler.protocol.bind<fb::game::handler::protocol::user_list>();       // User list handler
    this->handler.protocol.bind<fb::game::handler::protocol::chat>();            // User chat handler
    this->handler.protocol.bind<fb::game::handler::protocol::bulletin>();        // Bulletin section list handler
    this->handler.protocol.bind<fb::game::handler::protocol::swap>();            // Spell order change
    this->handler.protocol.bind<fb::game::handler::protocol::dialog>();          // Dialog
    this->handler.protocol.bind<fb::game::handler::protocol::dialog>(0x39);      // Dialog
    this->handler.protocol.bind<fb::game::handler::protocol::item_throws>();     // Item throw handler
    this->handler.protocol.bind<fb::game::handler::protocol::spell_cast>();      // Spell handler
    this->handler.protocol.bind<fb::game::handler::protocol::door>();            // Door handler
    this->handler.protocol.bind<fb::game::handler::protocol::whisper>();         // Whisper handler
    this->handler.protocol.bind<fb::game::handler::protocol::map_world>();       // World map handler
    this->handler.protocol.bind<fb::game::handler::protocol::miss>();
    this->handler.protocol.bind<fb::game::handler::protocol::give_item>();
    this->handler.protocol.bind<fb::game::handler::protocol::give_money>();
    this->handler.protocol.bind<fb::game::handler::protocol::post>();
    this->handler.protocol.bind<fb::game::handler::protocol::friends>();
    this->handler.protocol.bind<fb::game::handler::protocol::pong>();

    this->bind_timer<fb::game::handler::timer::heart_beat>(1s);
    this->bind_timer<fb::game::handler::timer::update_time>(1s);
    this->bind_timer<fb::game::handler::timer::schedule_timer>(1s);
    this->bind_timer<fb::game::handler::timer::system_mail_timer>(1s);

    this->initialize_schedules();
    auto announce_interval = std::chrono::seconds(fb::model::const_value::time::ANNOUNCE.total_milliseconds() / 1000);
    this->bind_timer<fb::game::handler::timer::announce>(announce_interval);
    this->bind_thread_timer<fb::game::handler::timer::mob_action_timer>(100ms);
    this->bind_thread_timer<fb::game::handler::timer::mob_respawn_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::buff_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::gear_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::soliloquy_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::save_timer>(std::chrono::seconds(fb::config<uint32_t>("save")));
    this->bind_thread_timer<fb::game::handler::timer::marketplace_restore_timer>(30s);
#if !defined(DEBUG) && !defined(_DEBUG)
    this->bind_thread_timer<fb::game::handler::timer::ping_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::afk_timer>(1s);
#endif

    auto world     = config<uint32_t>("world");
    auto host_name = std::format("fb.{}.game.{}", world, config<uint32_t>("id"));
    this->handler.amqp.bind<fb::game::handler::amqp::kick_out>(host_name);
    this->handler.amqp.bind<fb::game::handler::amqp::whisper>(host_name);
    this->handler.amqp.bind<fb::game::handler::amqp::storage_pending_personal>(host_name);
    this->handler.amqp.bind<fb::game::handler::amqp::shutdown>("fb.global"); // Shutdown: all servers
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast>(std::format("fb.{}.global", world));
    this->handler.amqp.bind<fb::game::handler::amqp::storage_pending_fetch>(std::format("fb.{}.global", world));
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast_save>(std::format("fb.{}.system", world));
    this->handler.amqp.bind<fb::game::handler::amqp::create_group>(std::format("fb.{}.group", world));
    this->handler.amqp.bind<fb::game::handler::amqp::updated_group>(std::format("fb.{}.group", world));
    this->handler.amqp.bind<fb::game::handler::amqp::destroy_group>(std::format("fb.{}.group", world));
    this->handler.amqp.bind<fb::game::handler::amqp::create_clan>(std::format("fb.{}.clan", world));
    this->handler.amqp.bind<fb::game::handler::amqp::destroy_clan>(std::format("fb.{}.clan", world));
    this->handler.amqp.bind<fb::game::handler::amqp::updated_clan>(std::format("fb.{}.clan", world));
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast_clan>(std::format("fb.{}.clan", world));
    this->handler.amqp.bind<fb::game::handler::amqp::write_mail>(std::format("fb.{}.mail", world));
    this->handler.amqp.bind<fb::game::handler::amqp::write_mails>(std::format("fb.{}.mail", world));
    this->handler.amqp.bind<fb::game::handler::amqp::deliver_system_mail>(std::format("fb.{}.mail", world));
    this->handler.amqp.bind<fb::game::handler::amqp::ban>(std::format("fb.{}.ban", world));
    this->handler.amqp.bind<fb::game::handler::amqp::set_exp_multiplier>(std::format("fb.{}.global", world));
    this->handler.amqp.bind<fb::game::handler::amqp::set_drop_rate_multiplier>(std::format("fb.{}.global", world));
    this->handler.amqp.bind<fb::game::handler::amqp::set_datetime>(std::format("fb.{}.global", world));
    this->handler.amqp.bind<fb::game::handler::amqp::start_maintenance>(
        std::format("fb.{}.game.{}", world, fb::config<uint32_t>("id")));

    // Fetch storage pending on server startup
    co_await this->storage_pending.fetch();

    // Run server init script once (gv and other globals) on the least loaded thread
    auto* init_thread = this->threads.least_loaded();
    if (init_thread != nullptr)
    {
        co_await init_thread->dispatch([this](auto&) -> async::task<void> {
            auto lua = fb::lua::new_context();
            if (lua != nullptr)
            {
                try
                {
                    lua->load("scripts/init.lua");
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("Server init script failed: {}", e.what());
                }
            }
            co_return;
        });
    }
}

bool server::decrypt_policy(uint8_t opcode) const
{
    switch (opcode)
    {
    case game_reqs::login::opcode:
        return false;

    default:
        return true;
    }
}

bool server::assert_tps(const fb::socket<fb::game::character>& socket) const
{
    auto ch = socket.data();
    if (ch == nullptr)
        return true;

    return ch->role() == ROLE::USER;
}

async::task<bool> server::on_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<bool> server::on_disconnected(fb::socket<character>& socket)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;

    auto weak = ch->weak_from_this_as<character>();

    if (ch->trade.trading())
        ch->trade.cancel();

    fb::logger::info("{} has disconnected.", ch->name());

    auto thread = ch->thread();
    try
    {
        if (!ch->saved_before_shutdown())
        {
            co_await this->save(*ch);
        }
        auto world  = fb::config<uint32_t>("world");
        std::ignore = co_await this->http.post("internal", "/in-game/logout", internal_reqs::Logout{world, ch->name()});
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Check if character is still valid before thread switching
    auto switched = false;
    try
    {
        co_await this->threads.switching(weak);
        switched = true;
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Thread switching failed during disconnect: {}", e.what());
    }

    if (!switched)
        co_await thread->switching();

    auto ptr = weak.lock();
    if (ptr != nullptr)
    {
        // Log logout event
        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
        log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
        log_data["level"]          = ptr->level();
        auto map                   = ptr->map();
        if (map != nullptr)
        {
            log_data["map"]        = map->model.id;
            log_data["position_x"] = ptr->position().x;
            log_data["position_y"] = ptr->position().y;
        }
        this->log.write("logout", log_data);

        auto& group_id = ptr->group_id();
        if (group_id.has_value())
        {
            this->groups.write(group_id.value(), [weak](auto& group) {
                group->detach(weak);
            });
            ptr->group_reset();
        }

        auto& clan_id = ptr->clan_id();
        if (clan_id.has_value())
        {
            this->clans.read(clan_id.value(), [weak](auto& clan) {
                clan->detach(weak);
            });
            ptr->clan_reset();
        }

        this->characters.write([ptr](auto& container) {
            container.remove(ptr);
        });
        co_await ch->destroy();
        socket.data(nullptr);
    }
    else
    {
        fb::logger::debug("Character expired during cleanup, skipping group/clan operations");
    }

    co_return true;
}

uint8_t server::id() const
{
    return fb::config<uint8_t>("id");
}

internal::Service server::service() const
{
    return internal::Service::Game;
}

async::task<void>
server::send(object& object, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self, bool encrypt)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    co_await header.serialize(writer);

    switch (scope)
    {
    case fb::game::scope::PIVOT:
    {
        if (!exclude_self)
            object.send(stream, encrypt);

        for (auto& x : object.nears(OBJECT_TYPE::CHARACTER, true))
        {
            if (x->sight(object) == false)
                continue;

            if (object.hidden(*x))
                continue;

            x->send(stream, encrypt);
        }
    }
    break;

    case fb::game::scope::GROUP:
    {
        if (object.is(OBJECT_TYPE::CHARACTER) == false)
            co_return;

        auto& ch       = static_cast<const character&>(object);
        auto& group_id = ch.group_id();
        if (group_id.has_value() == false)
            co_return;

        this->groups.read(group_id.value(), [&stream, encrypt](auto& group) {
            for (auto& shared_ptr : group->characters())
            {
                shared_ptr->send(stream, encrypt);
            }
        });
    }
    break;

    case fb::game::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            co_return;

        for (const auto& [seq, obj] : map->objects)
        {
            if (exclude_self && obj->oid() == object.oid())
                continue;

            obj->send(stream, encrypt);
        }
    }
    break;

    case fb::game::scope::WORLD:
    {
        this->characters.write([stream, encrypt](auto& characters) {
            characters.foreach_enqueue([stream, encrypt](auto& ch) -> async::task<void> {
                std::ignore = ch->send(stream, encrypt);
                co_return;
            });
        });
    }
    break;
    }
}

internal::SavePayload server::save_payload(const character& ch) const
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

        spells.push_back(internal::Spell{ch.id, i, spell->model.id, spell->next().to_string()});
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

    auto        now             = this->now();
    auto        storage_boxes   = std::vector<internal::StorageBox>();
    const auto& character_boxes = ch.storage_box.entries();
    storage_boxes.reserve(character_boxes.size());
    for (const auto& [id, box] : character_boxes)
    {
        if (box.expire_date.has_value() && box.expire_date.value() < now)
            continue;

        std::string attachments_json;
        if (box.attachments.empty())
        {
            attachments_json = "[]";
        }
        else
        {
            auto json_array = Json::Value{Json::arrayValue};
            for (const auto& dsl : box.attachments)
            {
                json_array.append(dsl.to_json());
            }
            auto builder           = Json::StreamWriterBuilder{};
            builder["emitUTF8"]    = true;
            builder["indentation"] = "";
            auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
            auto stream            = std::ostringstream{};
            writer->write(json_array, &stream);
            attachments_json = stream.str();
        }

        storage_boxes.emplace_back(ch.id,
                                   box.id,
                                   box.title,
                                   box.message,
                                   attachments_json,
                                   box.received,
                                   box.expire_date.has_value() ? std::make_optional(box.expire_date->to_string())
                                                               : std::nullopt);
    }

    auto        storage_reward_marks = std::vector<internal::StorageRewardMark>();
    const auto& reward_marks         = ch.storage_box.reward_marks();
    storage_reward_marks.reserve(reward_marks.size());
    for (const auto& [pending_id, mark] : reward_marks)
    {
        auto expired_date_str = std::optional<std::string>();
        if (mark.expire_date.has_value())
            expired_date_str = std::make_optional(mark.expire_date->to_string());
        storage_reward_marks.emplace_back(mark.user, pending_id, expired_date_str);
    }

    return internal::SavePayload(ch.to_protocol(),
                                 ch.marriage().to_protocol(),
                                 items,
                                 spells,
                                 achievements,
                                 quests,
                                 storage_boxes,
                                 storage_reward_marks);
}

async::task<void> server::save(character& ch)
{
    if (ch.inited() == false)
        co_return;

    auto weak    = ch.weak_from_this();
    auto world   = fb::config<uint32_t>("world");
    auto payload = this->save_payload(ch);
    std::ignore  = co_await this->http.post("internal", "/in-game/save", internal_reqs::Save{world, payload});

    co_await this->threads.switching(weak);
    ch.send(game_resp::save());
}

async::task<void> server::save()
{
    static constexpr size_t SAVE_BATCH_CHUNK_SIZE = 100;
    auto                    world                 = fb::config<uint32_t>("world");
    auto                    tasks                 = std::vector<async::task<void>>{};
    tasks.reserve(this->threads.count());
    for (auto& [id, thread] : this->threads)
    {
        tasks.push_back(thread->dispatch([this, world](auto& thread) -> async::task<void> {
            auto params     = thread.template data<thread_params>();
            auto characters = std::vector<character*>{};
            auto payloads   = std::vector<internal::SavePayload>{};
            characters.reserve(params->characters.size());
            payloads.reserve(params->characters.size());
            for (auto& [cid, character] : params->characters)
            {
                if (!character->inited())
                    continue;

                characters.push_back(character.get());
                payloads.push_back(this->save_payload(*character));
            }

            const size_t total = payloads.size();
            for (size_t offset = 0; offset < total; offset += SAVE_BATCH_CHUNK_SIZE)
            {
                const size_t chunk_end = (std::min)(offset + SAVE_BATCH_CHUNK_SIZE, total);
                auto         chunk =
                    std::vector<internal::SavePayload>(payloads.begin() + static_cast<std::ptrdiff_t>(offset),
                                                       payloads.begin() + static_cast<std::ptrdiff_t>(chunk_end));
                std::ignore = co_await this->http.post("internal",
                                                       "/in-game/save-batch",
                                                       internal_reqs::SaveBatch{world, std::move(chunk)});

                for (size_t i = offset; i < chunk_end; i++)
                {
                    characters[i]->send(game_resp::save());
                }
            }
            co_return;
        }));
    }
    for (auto& t : tasks)
    {
        co_await t;
    }
}

async::task<void> server::set_saved_before_shutdown_on_all()
{
    std::vector<async::task<void>> tasks;
    tasks.reserve(this->threads.count());
    for (auto& [id, thread] : this->threads)
    {
        tasks.push_back(thread->dispatch([this](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& [cid, character] : params->characters)
            {
                if (character->inited())
                    character->saved_before_shutdown(true);
            }
            co_return;
        }));
    }
    for (auto& t : tasks)
        co_await t;
}

async::task<void> server::on_exit()
{
    co_await this->set_saved_before_shutdown_on_all();
    co_await this->save();
}

uint32_t server::thread_id(const fb::socket<character>& socket) const
{
    auto character = socket.data();
    if (character == nullptr)
        throw std::runtime_error("character is empty");

    auto map = character->map();
    if (map == nullptr)
        return 0;

    return map->model.id;
}

fb::thread* server::thread(const map& map)
{
    auto count = this->threads.count();
    if (count == 0)
        return nullptr;

    return this->threads.at(map.model.id % count);
}

const fb::model::datetime& server::time() const
{
    return this->_time;
}

void server::on_init_amqp(fb::amqp::socket& amqp)
{
    auto world = config<uint32_t>("world");
    this->handler.amqp.declare_queue("amq.direct", "fb.global");                        // Shutdown: all servers
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.system", world)); // System mail, broadcast save
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.game.{}", world, fb::config<uint32_t>("id")));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.global", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.group", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.clan", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.mail", world));
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.ban", world));
}

async::task<void> server::broadcast(std::string_view message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type)
{
    auto message_str = std::string(message);
    switch (broadcast_type)
    {
    case BROADCAST_TYPE::GLOBAL:
    {
        auto   world = fb::config<uint32_t>("world");
        auto&& resp  = co_await this->http.post(
            "internal",
            "/in-game/broadcast",
            internal_reqs::Broadcast{world, fb::config<uint32_t>("id"), message_str, static_cast<uint8_t>(type)});
        co_await this->on_broadcast(resp);
    }
    break;

    case BROADCAST_TYPE::WORLD:
    {
        this->characters.write([message_str, type](auto& characters) {
            characters.foreach_enqueue([message_str, type](auto& ch) -> async::task<void> {
                ch->message(message_str, type);
                co_return;
            });
        });
    }
    break;
    }
}

async::task<void> server::on_broadcast(const internal_resp::Broadcast& resp)
{
    co_await this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type), BROADCAST_TYPE::WORLD);
}

void server::rezen_force()
{
    for (auto& [id, thread] : this->threads)
    {
        std::ignore = thread->dispatch([](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& rezen : params->rezens)
            {
                rezen.force_spawn(thread.id());
            }
            co_return;
        });
    }
}

void server::rezen_force(const fb::game::map& map)
{
    auto thread = map.thread();
    std::ignore = thread->dispatch([map_id = map.model.id](auto& thread) -> async::task<void> {
        auto params = thread.template data<thread_params>();
        for (auto& rezen : params->rezens)
        {
            if (rezen.model.parent == map_id)
                rezen.force_spawn(thread.id());
        }
        co_return;
    });
}

async::task<void> server::update_status()
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
                                                                        fb::config<uint16_t>("port")});
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to send heartbeat: {}", e.what());
    }
}

void server::update_time()
{
    auto updated = this->now();
    if (this->_time.hours() != updated.hours())
    {
        this->characters.write([hours = updated.hours()](auto& characters) {
            characters.foreach_enqueue([hours](auto& ch) -> async::task<void> {
                ch->update_time(hours);
                co_return;
            });
        });
    }

    this->_time = updated;
}

double server::exp_multiplier() const
{
    return this->_exp_multiplier;
}

void server::exp_multiplier(double value)
{
    this->_exp_multiplier = value;
}

double server::drop_rate_multiplier() const
{
    return this->_drop_rate_multiplier;
}

void server::drop_rate_multiplier(double value)
{
    this->_drop_rate_multiplier = value;
}

void server::initialize_schedules()
{
    auto now = this->now();

    for (const auto& schedule : table::schedule)
    {
        auto next = schedule.next_execution(now);
        if (next.has_value())
        {
            this->_scheduled_tasks[schedule.id] = next.value();
        }
    }
}

std::unordered_map<uint32_t, fb::model::datetime>& server::scheduled_tasks()
{
    return this->_scheduled_tasks;
}

void server::erase_map_cache(uint32_t map_id, const fb::model::point16_t& point)
{
    std::unique_lock lock(this->_map_update_cache_mutex);

    std::vector<uint64_t> to_erase;
    for (auto hash : this->_map_update_cache.keys())
    {
        const auto entry_map_id = static_cast<uint32_t>(hash >> 48);
        if (entry_map_id != map_id)
            continue;

        const uint16_t pos_x  = (hash >> 32) & 0xFFFF;
        const uint16_t pos_y  = (hash >> 16) & 0xFFFF;
        const uint8_t  width  = (hash >> 8) & 0xFF;
        const uint8_t  height = hash & 0xFF;
        auto           area   = fb::model::area<uint16_t>(pos_x, pos_y, pos_x + width, pos_y + height);
        if (area.contains(point))
            to_erase.push_back(hash);
    }

    for (uint64_t hash : to_erase)
    {
        this->_map_update_cache.erase(hash);
    }
}

void server::send_map_cache(character&                  ch,
                            const map&                  map,
                            const fb::model::point16_t& position,
                            const fb::model::size8_t&   size,
                            uint16_t                    crc)
{
    const auto hash = static_cast<uint64_t>(map.model.id) << 48 | static_cast<uint64_t>(position.x) << 32 |
                      static_cast<uint64_t>(position.y) << 16 | static_cast<uint64_t>(size.width) << 8 |
                      static_cast<uint64_t>(size.height);

    auto send_cache_bytes = [&ch, crc](const auto& cache_bytes) {
        if (cache_bytes.crc == crc)
            return;

        ch.send(fb::stream(cache_bytes.bytes.data(), cache_bytes.bytes.size()));
    };

    {
        std::shared_lock lock(this->_map_update_cache_mutex);
        if (this->_map_update_cache.try_read(hash, send_cache_bytes))
            return;
    }

    std::unique_lock lock(this->_map_update_cache_mutex);
    this->_map_update_cache.write(hash, send_cache_bytes, [&map, &position, &size, hash]() {
        auto bytes = map::cache_bytes();
        bytes.hash = hash;
        bytes.crc  = 0;

        auto writer = fb::stream_writer<big_endian>(bytes.bytes);
        auto resp   = game_resp::map_update(map, position, size);
        std::ignore = resp.serialize(writer);
        bytes.crc   = resp.crc;
        return bytes;
    });
}

void server::update_map_cache(uint32_t map_id, const fb::model::area<uint16_t>& area)
{
    // Phase 1: Invalidate cache entries whose region intersects the area (hold lock only for this).
    {
        std::unique_lock lock(this->_map_update_cache_mutex);

        std::vector<uint64_t> to_erase;
        for (auto hash : _map_update_cache.keys())
        {
            const auto entry_map_id = static_cast<uint32_t>(hash >> 48);
            if (entry_map_id != map_id)
                continue;

            const uint16_t e_left     = (hash >> 32) & 0xFFFF;
            const uint16_t e_top      = (hash >> 16) & 0xFFFF;
            const uint8_t  e_w        = (hash >> 8) & 0xFF;
            const uint8_t  e_h        = hash & 0xFF;
            const uint16_t e_right    = static_cast<uint16_t>(e_left + e_w);
            const uint16_t e_bottom   = static_cast<uint16_t>(e_top + e_h);
            const auto     entry_rect = fb::model::area<uint16_t>(e_left, e_top, e_right, e_bottom);

            if (entry_rect.intersects(area))
                to_erase.push_back(hash);
        }

        for (uint64_t hash : to_erase)
        {
            this->_map_update_cache.erase(hash);
        }
    }

    // Phase 2: Without holding the cache lock, collect characters whose view overlaps the area and send.
    auto map_ptr = maps.find(map_id);
    if (map_ptr == nullptr)
        return;

    const map& map     = *map_ptr;
    auto       viewers = std::vector<std::shared_ptr<character>>{};
    for (const auto& [fd, obj] : map.objects)
    {
        if (obj->is(OBJECT_TYPE::CHARACTER) == false)
            continue;

        if (obj->sight_area().intersects(area))
            viewers.push_back(std::static_pointer_cast<character>(obj));
    }

    const uint16_t             w = area.right > area.left ? static_cast<uint16_t>(area.right - area.left) : 0;
    const uint16_t             h = area.bottom > area.top ? static_cast<uint16_t>(area.bottom - area.top) : 0;
    const fb::model::point16_t begin(area.left, area.top);
    const fb::model::size8_t   size(static_cast<uint8_t>(w > 255 ? 255 : w), static_cast<uint8_t>(h > 255 ? 255 : h));

    for (const auto& ch : viewers)
    {
        ch->update_map(map, begin, size);
    }
}