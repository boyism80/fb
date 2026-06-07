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
    map(*this, fb::config<uint32_t>("id")),
    listener(*this),
    characters(*this),
    clan(*this),
    group(*this),
    mail(*this),
    bulletin(*this),
    system_storage(*this),
    system_mail(*this),
    schedules(*this),
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
    lua::build<fb::game::quest, lua::luable>();
    lua::build<fb::game::door, lua::luable>();
    lua::build<fb::game::clan, lua::luable>();
    lua::build<fb::game::clan_member, lua::luable>();
    lua::build<fb::game::achievement, lua::luable>();
    lua::build<fb::game::spell, lua::luable>();
    lua::build<fb::game::buff, lua::luable>();
    lua::build<fb::game::map, fb::thread_switchable>();
    lua::build<fb::game::group, fb::thread_switchable>();
    lua::build<fb::game::object, fb::thread_switchable>();
    lua::build<fb::game::life, fb::game::object>();
    lua::build<fb::game::mob, fb::game::life>();
    lua::build<fb::game::npc, fb::game::object>();
    lua::build<fb::game::item, fb::game::object>();
    lua::build<fb::game::equipment, fb::game::item>();
    lua::build<fb::game::weapon, fb::game::equipment>();
    lua::build<fb::game::character, fb::game::life>();
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
        auto& thread  = root->initial_thread();
        auto  builder = thread.new_builder<void>();
        builder.func  = [&root](auto&) -> async::task<void> {
            fb::model::lua::map_enum(*root);
            fb::model::lua::map_const(*root);
            co_return;
        };
        builder.enqueue();
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

    for (auto& [id, map] : this->map)
    {
        auto thread = this->threads.modular(id);
        maps_division[thread].push_back(map);
    }

    auto async_tasks = std::vector<async::task<void>>();
    for (auto& [thread, maps] : maps_division)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [this, maps = std::move(maps)](auto& thread) -> async::task<void> {
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
        };
        async_tasks.push_back(builder.dispatch());
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
    this->bind_timer<fb::game::handler::timer::system_storage_box_timer>(1s);

    this->schedules.init();
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
    this->handler.amqp.bind<fb::game::handler::amqp::shutdown>("fb.global"); // Shutdown: all servers
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast>(std::format("fb.{}.global", world));
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

    // Run server init script once (gv and other vars) on the least loaded thread
    auto* init_thread = this->threads.least_loaded();
    if (init_thread != nullptr)
    {
        auto builder = init_thread->new_builder<void>();
        builder.func = [this](auto&) -> async::task<void> {
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
        };
        co_await builder.dispatch();
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
            this->group.detach(weak, group_id.value());
            ptr->group_reset();
        }

        auto& clan_id = ptr->clan_id();
        if (clan_id.has_value())
        {
            this->clan.detach(weak, clan_id.value());
            ptr->clan_reset();
        }

        {
            auto guard = this->characters.enter_write();
            guard.value().remove(ptr);
        }
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

        {
            auto  guard = this->group.enter_read(group_id.value());
            auto& group = guard.value();
            for (auto& shared_ptr : group->characters())
            {
                shared_ptr->send(stream, encrypt);
            }
        }
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
        auto guard = this->characters.enter_write();
        guard.value().send(stream, encrypt);
    }
    break;
    }
}

void server::sync_time()
{
    auto updated = this->now();
    if (this->_time.hours() != updated.hours())
    {
        auto guard = this->characters.enter_write();
        guard.value().update_time(updated.hours());
    }

    this->_time = updated;
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

async::task<internal_resp::Ban> server::ban(std::string_view               name,
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

async::task<internal_resp::Unban> server::unban(std::string_view name)
{
    auto   name_str = std::string(name);
    auto   world    = fb::config<uint32_t>("world");
    auto&& resp     = co_await this->http.post("internal", "/ban/remove", internal_reqs::Unban{world, name_str});
    co_return std::move(resp);
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

    const auto now           = this->now();
    auto       storage_boxes = ch.storage_box.to_save_dtos(ch.id, now);

    return internal::SavePayload(ch.to_protocol(),
                                 ch.marriage().to_protocol(),
                                 items,
                                 spells,
                                 achievements,
                                 quests,
                                 storage_boxes);
}

async::task<void> server::save()
{
    static constexpr size_t SAVE_BATCH_CHUNK_SIZE = 100;
    auto                    world                 = fb::config<uint32_t>("world");
    auto                    tasks                 = std::vector<async::task<void>>{};
    tasks.reserve(this->threads.count());
    for (auto& [id, thread] : this->threads)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [this, world](auto& thread) -> async::task<void> {
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
        };
        tasks.push_back(builder.dispatch());
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
        auto builder = thread->new_builder<void>();
        builder.func = [this](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& [cid, character] : params->characters)
            {
                if (character->inited())
                    character->saved_before_shutdown(true);
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
