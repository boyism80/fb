#include <fb/game/server.h>
#include <fb/game/handler.h>
#include <fb/game/builtin/server.h>
using namespace fb::game;
using namespace std::chrono_literals;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<character>(io_context, "GAME", port),
    maps(*this, fb::config<uint32_t>("id")),
    _redis(config<std::string>("redis:ip").c_str(), config<uint16_t>("redis:port"), config<uint32_t>("redis:pool")),
    listener(*this),
    characters(*this),
    clans([](const std::shared_ptr<clan>& clan) -> uint32_t {
        return clan->id();
    }),
    groups([](const std::shared_ptr<group>& group) -> uint32_t {
        return group->id();
    }),
    map_update_cache(
        [](const map::cache_bytes& cache_bytes) -> uint64_t {
            return cache_bytes.hash;
        },
        1024)
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
    lua::build<fb::model::quest, lua::luable>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::achievement, lua::luable>();
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
    lua::build("baram_time", builtin::server::builtin_baram_time);
    lua::build("name2mob", builtin::server::builtin_name2mob);
    lua::build("name2spell", builtin::server::builtin_name2spell);
    lua::build("name2item", builtin::server::builtin_name2item);
    lua::build("name2npc", builtin::server::builtin_name2npc);
    lua::build("name2map", builtin::server::builtin_name2map);
    lua::build("name2ch", builtin::server::builtin_name2ch);
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

    for (auto& [_, root] : ist)
    {
        auto& thread = root->initial_thread();
        std::ignore  = thread.dispatch([root](auto&) -> async::task<void> {
            fb::model::lua::map_enum(*root);
            co_return;
        });
    }
}

server::~server()
{ }

async::task<void> server::handle_start()
{
    this->threads.deletor = [](void* data) {
        auto params = static_cast<thread_params*>(data);
        delete params;
    };

    co_await fb::acceptor<character>::handle_start();

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
            auto params = new thread_params();
            for (auto map : maps)
            {
                params->maps.insert({map->model.id, map});
                if (this->model.mob_spawn.contains(map->model.id))
                {
                    for (auto& spawn : this->model.mob_spawn[map->model.id])
                    {
                        params->rezens.push_back(fb::game::rezen(*this, spawn));
                    }
                }
            }
            thread.data(params);
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
    this->handler.protocol.bind<fb::game::handler::protocol::attack>();          // Attack handler
    this->handler.protocol.bind<fb::game::handler::protocol::loot>();            // Loot handler
    this->handler.protocol.bind<fb::game::handler::protocol::emotion>();         // Emotion handler
    this->handler.protocol.bind<fb::game::handler::protocol::map_update>();      // Map data update handler
    this->handler.protocol.bind<fb::game::handler::protocol::update_screen>();   // Refresh handler
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

    this->bind_timer<fb::game::handler::timer::heart_beat>(1s);
    this->bind_timer<fb::game::handler::timer::update_time>(1s);
    this->bind_timer<fb::game::handler::timer::announce>(std::chrono::seconds(fb::model::const_value::time::ANNOUNCE.total_milliseconds() / 1000));

    this->bind_thread_timer<fb::game::handler::timer::mob_action_timer>(100ms);
    this->bind_thread_timer<fb::game::handler::timer::mob_respawn_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::buff_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::gear_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::soliloquy_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::save_timer>(std::chrono::seconds(fb::config<uint32_t>("save")));

    this->bind_npc_interaction<fb::game::handler::npc_interaction::sell>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::buy>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::repair>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::deposit_money>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::withdraw_money>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::store_item>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::retrieve_item>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::sell_list>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::buy_list>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::sell_price>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::buy_price>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::show_deposited_money>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::rename_weapon>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::store_item_list>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::store_item_count>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::revive>();
    this->bind_npc_interaction<fb::game::handler::npc_interaction::appreciate>();

    this->handler.amqp.bind<fb::game::handler::amqp::kick_out>(std::format("fb.game.{}", config<uint32_t>("id")));
    this->handler.amqp.bind<fb::game::handler::amqp::whisper>(std::format("fb.game.{}", config<uint32_t>("id")));
    this->handler.amqp.bind<fb::game::handler::amqp::shutdown>("fb.system");
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast>("fb.global");
    this->handler.amqp.bind<fb::game::handler::amqp::enter_group>("fb.group");
    this->handler.amqp.bind<fb::game::handler::amqp::leave_group>("fb.group");
    this->handler.amqp.bind<fb::game::handler::amqp::kick_group>("fb.group");
    this->handler.amqp.bind<fb::game::handler::amqp::set_clan_title>("fb.clan");
    this->handler.amqp.bind<fb::game::handler::amqp::join_clan>("fb.clan");
    this->handler.amqp.bind<fb::game::handler::amqp::leave_clan>("fb.clan");
    this->handler.amqp.bind<fb::game::handler::amqp::kick_clan>("fb.clan");
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast_clan>("fb.clan");
    this->handler.amqp.bind<fb::game::handler::amqp::write_mail>("fb.mail");
}

bool server::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x10:
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

async::task<bool> server::handle_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<bool> server::handle_disconnected(fb::socket<character>& socket)
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
        co_await this->save(*ch);
        std::ignore = co_await this->http.post("internal", "/in-game/logout", Logout{ch->name()});
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

        this->characters.remove(ptr);
        co_await ch->destroy();
        socket.data(nullptr);
    }
    else
    {
        fb::logger::debug("Character expired during cleanup, skipping group/clan operations");
    }

    co_return true;
}

std::shared_ptr<fb::game::character> server::handle_accepted(fb::socket<character>& socket)
{
    return this->make<character>(socket);
}

uint8_t server::id() const
{
    return fb::config<uint8_t>("id");
}

Service server::service() const
{
    return Service::Game;
}

async::task<void> server::send(object& object, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self, bool encrypt)
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
        co_await this->characters.foreach ([stream, encrypt](auto& ch) {
            ch->send(stream, encrypt);
        });
    }
    break;
    }
}

async::task<void> server::save(character& ch)
{
    if (ch.inited() == false)
        co_return;

    auto weak  = ch.weak_from_this();
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

        spells.push_back(internal::Spell{ch.id(), i, spell->model.id, spell->next().to_string()});
    }

    auto achievements = std::vector<internal::Achievement>();
    for (auto& [model, achievement] : ch.achievements)
    {
        achievements.push_back(internal::Achievement{ch.id(), model, achievement->text, achievement->icon, achievement->color});
    }

    auto quests = std::vector<internal::Quest>();
    for (auto& [qid, quest] : ch.quests)
    {
        quests.push_back(internal::Quest{ch.id(), qid, quest->step(), quest->progress(), quest->param(), quest->completed()});
    }

    std::ignore = co_await this->http.post("internal", "/in-game/save", Save{ch.to_protocol(), items, spells, achievements, quests});

    co_await this->threads.switching(weak);
    ch.send(fb_resp::save());
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

void server::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.game.{}", fb::config<uint32_t>("id")));
    this->handler.amqp.declare_queue("amq.direct", "fb.global");
    this->handler.amqp.declare_queue("amq.direct", "fb.group");
    this->handler.amqp.declare_queue("amq.direct", "fb.clan");
    this->handler.amqp.declare_queue("amq.direct", "fb.mail");
}

async::task<void> server::broadcast(const std::string& message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type)
{
    switch (broadcast_type)
    {
    case BROADCAST_TYPE::GLOBAL:
    {
        auto&& resp = co_await this->http.post("internal", "/in-game/broadcast", Broadcast{fb::config<uint32_t>("id"), message, static_cast<uint8_t>(type)});
        this->on_broadcast(resp);
    }
    break;

    case BROADCAST_TYPE::WORLD:
    {
        co_await this->characters.foreach ([message, type](auto& ch) {
            ch->message(message, type);
        });
    }
    break;
    }
}

void server::on_broadcast(const internal_resp::Broadcast& resp)
{
    std::ignore = this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type), BROADCAST_TYPE::WORLD);
}

void server::rezen_force()
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
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

void server::update_status()
{
    auto root    = Json::Value{};
    root["Name"] = this->name();
    root["IP"]   = fb::config<std::string>("ip");
    root["Port"] = fb::config<uint16_t>("port");
    auto writer  = Json::FastWriter{};
    auto output  = writer.write(root);

    this->_redis.command<void>(std::format("SET heart-beat:Game:{} {}", this->id(), output));
    this->_redis.command<void>(std::format("EXPIRE heart-beat:Game:{} 5", this->id()));
}

void server::update_time()
{
    auto updated = fb::model::datetime();
    if (this->_time.hours() != updated.hours())
    {
        this->characters.foreach ([hours = updated.hours()](auto& ch) {
            ch->update_time(hours);
        });
    }

    this->_time = updated;
}

async::task<bool> server::npc_interaction(character& ch, const std::string& message, const std::vector<std::shared_ptr<fb::game::npc>>& npcs)
{
    ch.assert_thread();

    if (npcs.size() == 0)
        co_return false;

    // Try new handler system first
    for (auto& handler : this->_npc_interaction_handlers)
    {
        if (handler->matches(message))
        {
            co_await handler->handle(ch, message, npcs);
            co_return true;
        }
    }

    co_return false;
}