#include <fb/game/server.h>
#include <fb/game/handler.h>
#include <fb/game/builtin/server.h>
#include <fb/game/worker.h>
#include <fb/lua.h>
#include <fb/encoding.h>
#include <fb/console.h>
#include <fb/logger.h>
#include <fb/amqp_route.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <format>
#include <boost/asio/post.hpp>

using namespace fb::game;
using namespace fb::model::enum_value;
using namespace std::chrono_literals;
using table = fb::model::table;

async::task<void> fb::game::server::init_lua()
{
    for (auto& [_, root] : this->lua)
    {
        co_await root->switching();

        auto& lua = *root;
        lua.build<fb::lua::luable>();
        lua.build<fb::thread, fb::lua::luable>();
        lua.build<fb::thread_switchable, fb::lua::luable>();

        lua.build<fb::game::quest, fb::lua::luable>();
        lua.build<fb::game::door, fb::lua::luable>();
        lua.build<fb::game::clan, fb::lua::luable>();
        lua.build<fb::game::castle, fb::lua::luable>();
        lua.build<fb::game::clan_member, fb::lua::luable>();
        lua.build<fb::game::achievement, fb::lua::luable>();
        lua.build<fb::game::spell, fb::lua::luable>();
        lua.build<fb::game::buff, fb::lua::luable>();
        lua.build<fb::game::map, fb::thread_switchable>();
        lua.build<fb::game::matchmaker, fb::lua::luable>();
        lua.build<fb::game::match, fb::lua::luable>();
        lua.build<fb::game::group, fb::thread_switchable>();
        lua.build<fb::game::object, fb::thread_switchable>();
        lua.build<fb::game::life, fb::game::object>();
        lua.build<fb::game::mob, fb::game::life>();
        lua.build<fb::game::npc, fb::game::object>();
        lua.build<fb::game::item, fb::game::object>();
        lua.build<fb::game::equipment, fb::game::item>();
        lua.build<fb::game::weapon, fb::game::equipment>();
        lua.build<fb::game::character, fb::game::life>();
        lua.build<fb::model::spell, fb::lua::luable>();
        lua.build<fb::model::map, fb::lua::luable>();
        lua.build<fb::model::object, fb::lua::luable>();
        lua.build<fb::model::life, fb::model::object>();
        lua.build<fb::model::mob, fb::model::life>();
        lua.build<fb::model::npc, fb::model::object>();
        lua.build<fb::model::item, fb::model::object>();
        lua.build<fb::model::equipment, fb::model::item>();
        lua.build<fb::model::weapon, fb::model::equipment>();

        lua.build("log", builtin::server::builtin_log);
        lua.build("seed", builtin::server::builtin_seed);
        lua.build("sleep", builtin::server::builtin_sleep);
        lua.build("now", builtin::server::builtin_now);
        lua.build("time_forward", builtin::server::builtin_time_forward);
        lua.build("time_backward", builtin::server::builtin_time_backward);
        lua.build("datetime", builtin::server::builtin_datetime);
        lua.build("to_lunar", builtin::server::builtin_to_lunar);
        lua.build("from_lunar", builtin::server::builtin_from_lunar);
        lua.build("event_is", builtin::server::builtin_event_is);
        lua.build("name2mob", builtin::server::builtin_name2mob);
        lua.build("name2spell", builtin::server::builtin_name2spell);
        lua.build("name2item", builtin::server::builtin_name2item);
        lua.build("name2npc", builtin::server::builtin_name2npc);
        lua.build("name2map", builtin::server::builtin_name2map);
        lua.build("name2ch", builtin::server::builtin_name2ch);
        lua.build("id2mob", builtin::server::builtin_id2mob);
        lua.build("id2spell", builtin::server::builtin_id2spell);
        lua.build("id2item", builtin::server::builtin_id2item);
        lua.build("id2npc", builtin::server::builtin_id2npc);
        lua.build("id2map", builtin::server::builtin_id2map);
        lua.build("id2ch", builtin::server::builtin_id2ch);
        lua.build("id2clan", builtin::server::builtin_id2clan);
        lua.build("castle", builtin::server::builtin_castle);
        lua.build("siege_active", builtin::server::builtin_siege_active);
        lua.build("broadcast", builtin::server::builtin_broadcast);
        lua.build("assert_alive", builtin::server::builtin_assert_alive);
        lua.build("pursuit_sell", builtin::server::builtin_pursuit_sell);
        lua.build("pursuit_sell_price", builtin::server::builtin_pursuit_sell_price);
        lua.build("pursuit_buy", builtin::server::builtin_pursuit_buy);
        lua.build("timer", builtin::server::builtin_timer);
        lua.build("weather", builtin::server::builtin_weather);
        lua.build("weather_reroll", builtin::server::builtin_weather_reroll);
        lua.build("bright", builtin::server::builtin_bright);
        lua.build("name_with", builtin::server::builtin_name_with);
        lua.build("assert_korean", builtin::server::builtin_assert_korean);
        lua.build("CP949", builtin::server::builtin_cp949);
        lua.build("debug", builtin::server::builtin_debug);
        lua.build("name2class", builtin::server::builtin_name2class);
        lua.build("class2name", builtin::server::builtin_class2name);
        lua.build("save", builtin::server::builtin_save);
        lua.build("reload_table", builtin::server::builtin_reload_table);
        lua.build("mknpc", builtin::server::builtin_mknpc);
        lua.build("maps", builtin::server::builtin_maps);
        lua.build("shutdown", builtin::server::builtin_shutdown);
        lua.build("ban", builtin::server::builtin_ban);
        lua.build("unban", builtin::server::builtin_unban);
        lua.build("regex", builtin::server::builtin_regex);
        lua.build("exp_multiplier", builtin::server::builtin_exp_multiplier);
        lua.build("drop_rate_multiplier", builtin::server::builtin_drop_rate_multiplier);
        lua.build("http_response_delay", builtin::server::builtin_http_response_delay);
        lua.build("property", builtin::server::builtin_property);
        lua.build("match_transfer", builtin::server::builtin_match_transfer);
        lua.build("is_cross", builtin::server::builtin_is_cross);

        fb::model::lua::map_enum(lua);
        fb::model::lua::map_const(lua);

        {
            lua_State* L = lua;
            lua_newtable(L);
            lua_pushinteger(L, static_cast<lua_Integer>(fb::game::MOB_PARTS_MODE::PARTS));
            lua_setfield(L, -2, "PARTS");
            lua_pushinteger(L, static_cast<lua_Integer>(fb::game::MOB_PARTS_MODE::BODY));
            lua_setfield(L, -2, "BODY");
            lua_setglobal(L, "MOB_PARTS_MODE");
        }

        lua.package_path("scripts/?.lua");
        lua.package_path("scripts/?/init.lua");
    }
}

async::task<void> fb::game::server::init_thread_params()
{
    auto maps_division = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::game::map>>>{};
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        maps_division.insert({thread, std::vector<std::shared_ptr<fb::game::map>>{}});
    }

    for (const auto& map : *this->maps.snapshot())
    {
        auto thread = this->threads.modular(map->id);
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
                params->add_map(map);
                if (table::mob_spawn->contains(map->model().id))
                {
                    auto& spawns = table::mob_spawn[map->model().id];
                    for (uint32_t i = 0; i < spawns.size(); i++)
                    {
                        params->rezens.push_back(std::make_unique<fb::game::rezen>(*this, map->model().id, i, map));
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
        co_await async_task;
    }
    co_return;
}

void fb::game::server::init_handlers()
{
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
    this->handler.protocol.bind<fb::game::handler::protocol::dialog_list>();     // Dialog list
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
    this->handler.protocol.bind<fb::game::handler::protocol::user_info_submit>();
    this->handler.protocol.bind<fb::game::handler::protocol::popup_input_submit>();
    this->handler.protocol.bind<fb::game::handler::protocol::collection>(); // 6.51 collection UI
    this->handler.protocol.bind<fb::game::handler::protocol::unknown_54>(); // 5.65+ 0x4F window round trip
    this->handler.protocol.bind<fb::game::handler::protocol::browser>();    // 6.51 in-game IE window
    this->handler.protocol.bind<fb::game::handler::protocol::web_map>();    // 6.51 web map
}

void fb::game::server::init_timers()
{
    this->bind_timer<fb::game::handler::timer::heart_beat>(1s);
    this->bind_timer<fb::game::handler::timer::update_time>(1s);
    this->bind_timer<fb::game::handler::timer::schedule_timer>(1s);
    this->bind_timer<fb::game::handler::timer::system_mail_timer>(1s);
    this->bind_timer<fb::game::handler::timer::system_storage_box_timer>(1s);

    if (fb::is_cross() == false)
        this->schedules.init();
    auto announce_interval = std::chrono::seconds(fb::model::const_value::time::ANNOUNCE.total_milliseconds() / 1000);
    this->bind_timer<fb::game::handler::timer::announce>(announce_interval);
    this->bind_thread_timer<fb::game::handler::timer::mob_action_timer>(100ms);
    this->bind_thread_timer<fb::game::handler::timer::mob_respawn_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::buff_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::expired_item_timer>(10s);
    this->bind_thread_timer<fb::game::handler::timer::gear_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::soliloquy_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::save_timer>(std::chrono::seconds(fb::config<uint32_t>("save")));
    this->bind_thread_timer<fb::game::handler::timer::marketplace_restore_timer>(30s);
#if !defined(DEBUG) && !defined(_DEBUG)
    this->bind_thread_timer<fb::game::handler::timer::ping_timer>(1s);
    this->bind_thread_timer<fb::game::handler::timer::afk_timer>(1s);
#endif
}

void fb::game::server::init_amqp_handlers()
{
    auto scope = fb::amqp_scope();
    auto id    = fb::config<uint32_t>("id");
    auto game  = fb::amqp_key("game", scope, id);

    this->handler.amqp.bind<fb::game::handler::amqp::kick_out>(game);
    this->handler.amqp.bind<fb::game::handler::amqp::whisper>(game);
    this->handler.amqp.bind<fb::game::handler::amqp::friend_relation>(game);
    this->handler.amqp.bind<fb::game::handler::amqp::friend_message>(game);
    this->handler.amqp.bind<fb::game::handler::amqp::start_maintenance>(game);
    this->handler.amqp.bind<fb::game::handler::amqp::shutdown>("fb.global");

    this->handler.amqp.bind<fb::game::handler::amqp::create_clan>(fb::amqp_key("clan", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::destroy_clan>(fb::amqp_key("clan", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::updated_clan>(fb::amqp_key("clan", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast_clan>(fb::amqp_key("clan", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::create_group>(fb::amqp_key("group", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::updated_group>(fb::amqp_key("group", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::destroy_group>(fb::amqp_key("group", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::write_mail>(fb::amqp_key("mail", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::write_mails>(fb::amqp_key("mail", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::deliver_system_mail>(fb::amqp_key("mail", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::write_storage_box>(fb::amqp_key("storage", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::deliver_system_storage>(fb::amqp_key("storage", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::ban>(fb::amqp_key("ban", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::broadcast_save>(fb::amqp_key("system", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::matchmaking_proposed>(fb::amqp_key("matchmaking", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::matchmaking_ready>(fb::amqp_key("matchmaking", scope));
    this->handler.amqp.bind<fb::game::handler::amqp::matchmaking_dissolved>(fb::amqp_key("matchmaking", scope));

    if (scope != "cross")
    {
        this->handler.amqp.bind<fb::game::handler::amqp::updated_castle>(fb::amqp_key("castle", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::broadcast>(fb::amqp_key("global", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::set_datetime>(fb::amqp_key("global", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::set_exp_multiplier>(fb::amqp_key("global", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::set_drop_rate_multiplier>(fb::amqp_key("global", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::reload_tables>(fb::amqp_key("global", scope));
        this->handler.amqp.bind<fb::game::handler::amqp::reload_scripts>(fb::amqp_key("global", scope));
    }
}

async::task<void> fb::game::server::init_map_scripts()
{
    auto async_tasks = std::vector<async::task<void>>();
    for (const auto& map : *this->maps.snapshot())
    {
        if (map->active == false || map->loaded() == false)
            continue;

        async_tasks.push_back(this->maps.invoke_init_script_wait(map));
    }

    for (auto& async_task : async_tasks)
    {
        co_await async_task;
    }
    co_return;
}

async::task<void> fb::game::server::init_script()
{
    auto* init_thread = this->threads.least_loaded();
    if (init_thread == nullptr)
        co_return;

    auto builder = init_thread->new_builder<void>();
    builder.func = [this](auto&) -> async::task<void> {
        // init.lua may call yielding builtins (e.g. map:model); run via call()/lua_resume,
        // not load()/lua_pcall which cannot cross a C-call boundary.
        static constexpr auto path = "scripts/init.lua";
        auto                  lua  = this->lua.open();
        if (!lua)
            co_return;

        auto* ctx = lua.get();
        if (::luaL_loadfile(*ctx, path) != LUA_OK)
        {
            fb::lua::report_load_failed_from_stack(*ctx, path);
            fb::logger::warn("Server init script failed: {}", path);
            co_return;
        }

        try
        {
            std::ignore = co_await lua->call(0);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Server init script failed: {}: {}", path, e.what());
        }
        co_return;
    };
    co_await builder.dispatch();
}

fb::game::server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<character>(io_context, "GAME", port, fb::config<uint32_t>("http:max_concurrent", 128)),
    maps(*this, fb::config<uint32_t>("id")),
    listener(*this),
    characters(*this),
    clans(*this),
    castles(*this),
    groups(*this),
    matches(*this),
    mail(*this),
    bulletin(*this),
    system_storage(*this),
    system_mail(*this),
    schedules(*this),
    script_timers(*this),
    weather(*this),
    log(fb::config<std::string>("amqp:log:ip"),
        fb::config<uint16_t>("amqp:log:port"),
        fb::config<std::string>("amqp:log:uid"),
        fb::config<std::string>("amqp:log:pwd"),
        std::to_string(fb::config<uint32_t>("id")),
        fb::config<std::string>("name"),
        fb::config<uint32_t>("world")),
    _exp_multiplier(fb::config<double>("exp_multiplier")),
    _drop_rate_multiplier(fb::config<double>("drop_rate_multiplier"))
{ }

fb::game::server::~server()
{ }

async::task<void> fb::game::server::on_start()
{
#ifdef _WIN32
    fb::model::option::decoding(fb::cp949);
    fb::model::option::encoding(fb::utf8);
    flatbuffers::option::encoding(fb::utf8);
    flatbuffers::option::decoding(fb::cp949);
#endif

    table::npc->hook.build = [](const Json::Value& json) -> fb::model::npc* {
        auto clone    = json;
        clone["look"] = clone["look"].asUInt() + 0x7FFF;
        return fb::model::build<fb::model::npc*>(clone);
    };

    table::mob->hook.build = [](const Json::Value& json) -> fb::model::mob* {
        auto clone    = json;
        clone["look"] = clone["look"].asUInt() + 0x7FFF;
        return fb::model::build<fb::model::mob*>(clone);
    };

    table::item->hook.build = [](const Json::Value& json) -> fb::model::item* {
        auto clone    = json;
        clone["look"] = clone["look"].asUInt() + 0xBFFF;

        auto type = fb::model::build<ITEM_TYPE>(clone["type"]);
        switch (type)
        {
        case ITEM_TYPE::STUFF:
            return fb::model::build<fb::model::item*>(clone);
        case ITEM_TYPE::CASH:
            return fb::model::build<fb::model::cash*>(clone);
        case ITEM_TYPE::CONSUME:
            return fb::model::build<fb::model::consume*>(clone);
        case ITEM_TYPE::WEAPON:
            return fb::model::build<fb::model::weapon*>(clone);
        case ITEM_TYPE::ARMOR:
            return fb::model::build<fb::model::armor*>(clone);
        case ITEM_TYPE::HELMET:
            return fb::model::build<fb::model::helmet*>(clone);
        case ITEM_TYPE::RING:
            return fb::model::build<fb::model::ring*>(clone);
        case ITEM_TYPE::SHIELD:
            return fb::model::build<fb::model::shield*>(clone);
        case ITEM_TYPE::AUXILIARY:
            return fb::model::build<fb::model::auxiliary*>(clone);
        case ITEM_TYPE::PACKAGE:
            return fb::model::build<fb::model::pack*>(clone);
        default:
            return nullptr;
        }
    };

    co_await this->init_lua();
    co_await fb::model::loader(*this).run();
    this->meta.load(fb::config<std::string>("meta_dat", std::string("Meta.dat")), true);
    this->sobj.load(fb::config<std::string>("sobj_tbl", std::string("SObj.tbl")));
    this->init_collection_mobs();
    co_await map_loader(*this).run();
    co_await script_loader(*this).run();
    co_await npc_spawner(*this).run();

    fb::console::set_mode(fb::console::mode::plain);
#ifdef _WIN32
    fb::model::option::decoding(nullptr);
#endif

    co_await this->init_thread_params();
    co_await this->init_map_scripts();
    this->init_handlers();
    this->init_timers();
    this->weather.sync();
    this->init_amqp_handlers();
    // HTTP needs io_context, which starts only after on_start returns.
    // Queue castle warmup on the first game thread once IO is running.
    boost::asio::post(this->io_context, [this]() {
        auto* thread = this->threads.at(0);
        if (thread == nullptr)
        {
            fb::logger::warn("castles: no game thread available for startup load");
            return;
        }

        auto builder = thread->new_builder<void>();
        builder.func = [this](auto&) -> async::task<void> {
            try
            {
                if (fb::is_cross() == false)
                {
                    co_await this->castles.load_all();
                    fb::logger::info("castles: loaded all divine beast castles");
                }
            }
            catch (std::exception& e)
            {
                fb::logger::warn("castles: load_all failed: {}", e.what());
            }
            co_return;
        };
        builder.enqueue();
    });
    co_await this->init_script();
}