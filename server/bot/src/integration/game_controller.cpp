#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/lua_integration_test.h>
#include <fb/bot/integration/trade_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/container.h>
#include <fb/bot/gateway_controller.h>
#include <fb/logger.h>
#include <fb/config.h>
#include <async/awaitable_get.h>
#include <async/awaitable_then.h>

using namespace fb::bot::integration;
using table = fb::model::table;
using namespace fb::model::enum_value;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{
    this->bind<trade_bot>([](game_bot& bot, trade_bot& protocol) -> async::task<void> {
        co_return;
    });
    this->bind(&game_bot_controller::on_time);
    this->bind(&game_bot_controller::on_state);
    this->bind(&game_bot_controller::on_message);
    this->bind(&game_bot_controller::on_sequence);
    this->bind(&game_bot_controller::on_position);
    this->bind(&game_bot_controller::on_move);
    this->bind(&game_bot_controller::on_map);
    this->bind(&game_bot_controller::on_transfer);
}

void game_bot_controller::initialize()
{
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

    async::awaitable_get(fb::model::loader(this->container).run());

    this->bind_timer(&game_bot_controller::on_timer, 1000ms);

    this->_max_parallel_tests = fb::config<uint32_t>("integration:max_parallel_tests", 4u);
    if (this->_max_parallel_tests == 0)
        this->_max_parallel_tests = 1;

    // Instance slots must be >= 1 (see scripts/lib/command.lua 맵이동). Seats are 1..K;
    // tests that need a second instance use extra_slot = K+1.
    for (uint32_t i = 1; i <= this->_max_parallel_tests; i++)
        this->_free_seats.insert(i);

    for (auto& discovered : lua_integration_test::discover_scripts())
    {
        this->enqueue_test(
            std::make_unique<lua_integration_test>(*this, discovered.path, discovered.serial, discovered.extra_slot),
            discovered.serial,
            discovered.extra_slot);
    }

    fb::logger::info("Integration test controller initialized: parallel={} serial={} max_parallel={}",
                     this->_parallel_queue.size(),
                     this->_serial_queue.size(),
                     this->_max_parallel_tests);

    this->_suite_start = std::chrono::steady_clock::now();
    this->try_schedule_parallel();
    if (this->_active_seats.empty() && this->_parallel_queue.empty())
        this->start_serial_phase();
}

uint32_t game_bot_controller::max_parallel_tests() const
{
    return this->_max_parallel_tests;
}

void game_bot_controller::own(uint32_t bot_id, bot_integration_test* test)
{
    auto lock                 = std::lock_guard(this->_schedule_mutex);
    this->_bot_owners[bot_id] = test;
}

void game_bot_controller::reown(uint32_t old_bot_id, uint32_t new_bot_id)
{
    auto lock = std::lock_guard(this->_schedule_mutex);
    auto it   = this->_bot_owners.find(old_bot_id);
    if (it == this->_bot_owners.end())
        return;

    auto* test = it->second;
    this->_bot_owners.erase(it);
    this->_bot_owners[new_bot_id] = test;
}

bot_integration_test* game_bot_controller::owner_of(uint32_t bot_id)
{
    auto lock = std::lock_guard(this->_schedule_mutex);
    auto it   = this->_bot_owners.find(bot_id);
    if (it == this->_bot_owners.end())
        return nullptr;
    return it->second;
}

void game_bot_controller::clear_ownership_for_test(bot_integration_test* test)
{
    auto lock = std::lock_guard(this->_schedule_mutex);
    for (auto it = this->_bot_owners.begin(); it != this->_bot_owners.end();)
    {
        if (it->second == test)
            it = this->_bot_owners.erase(it);
        else
            ++it;
    }
}

void game_bot_controller::enqueue_test(std::unique_ptr<bot_integration_test> test, bool serial, bool extra_slot)
{
    test->serial(serial);
    test->needs_extra_slot(extra_slot);
    this->_test_instances.push_back(std::move(test));
    auto* ptr = this->_test_instances.back().get();
    if (serial)
        this->_serial_queue.push_back(ptr);
    else
        this->_parallel_queue.push_back(ptr);
}

void game_bot_controller::try_schedule_parallel()
{
    std::vector<bot_integration_test*> to_start;

    {
        auto lock = std::lock_guard(this->_schedule_mutex);
        while (this->_parallel_queue.empty() == false && this->_free_seats.empty() == false)
        {
            auto seat_it = this->_free_seats.begin();
            auto seat    = *seat_it;
            this->_free_seats.erase(seat_it);

            auto* test = this->_parallel_queue.front();
            this->_parallel_queue.pop_front();

            test->suite_slot(seat);
            if (test->needs_extra_slot())
                test->extra_slot(this->_max_parallel_tests + 1);
            else
                test->extra_slot(std::nullopt);

            this->_active_seats[test] = seat;
            to_start.push_back(test);
        }
    }

    for (auto* test : to_start)
        this->detach_run_one(test);
}

void game_bot_controller::detach_run_one(bot_integration_test* test)
{
    async::awaitable_then(this->run_one(test), [this, test](auto result) {
        try
        {
            result();
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("Test '{}' run_one failed: {}", test->name(), e.what());
            this->on_test_complete(test, false);
        }
        catch (...)
        {
            fb::logger::fatal("Test '{}' run_one failed", test->name());
            this->on_test_complete(test, false);
        }
    });
}

async::task<void> game_bot_controller::run_one(bot_integration_test* test)
{
    auto success      = false;
    auto need_cleanup = false;
    try
    {
        fb::logger::info("Starting test '{}' (suite_slot={})", test->name(), test->suite_slot());
        test->prepare_ready_wait();
        co_await test->on_activated(*this);
        co_await test->wait_until_ready();
        success = co_await test->execute();
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Test '{}' exception: {}", test->name(), e.what());
        success      = false;
        need_cleanup = true;
    }
    catch (...)
    {
        fb::logger::fatal("Test '{}' exception", test->name());
        success      = false;
        need_cleanup = true;
    }

    if (need_cleanup)
    {
        try
        {
            co_await test->on_finished();
        }
        catch (...)
        { }
    }

    this->on_test_complete(test, success);
    co_return;
}

void game_bot_controller::on_test_complete(bot_integration_test* test, bool success)
{
    {
        auto        lock = std::lock_guard(this->_results_mutex);
        test_result result;
        result.name    = test->name();
        result.success = success;
        result.message = success ? "PASSED" : "FAILED";
        this->_test_results.push_back(result);
    }

    if (success)
        fb::logger::info(fb::console::color::light_green, "Test '{}' completed successfully", test->name());
    else
        fb::logger::fatal(fb::console::color::light_red, "Test '{}' failed", test->name());

    this->clear_ownership_for_test(test);

    bool parallel_done     = false;
    bool start_serial_next = false;
    bool continue_serial   = false;
    bool finish            = false;

    {
        auto lock = std::lock_guard(this->_schedule_mutex);
        if (auto it = this->_active_seats.find(test); it != this->_active_seats.end())
        {
            this->_free_seats.insert(it->second);
            this->_active_seats.erase(it);
        }

        if (test->serial() && this->_serial_queue.empty() == false && this->_serial_queue.front() == test)
            this->_serial_queue.pop_front();

        parallel_done = this->_active_seats.empty() && this->_parallel_queue.empty();

        if (parallel_done == false)
        {
            // schedule more parallel outside lock
        }
        else if (this->_serial_phase_started == false)
        {
            start_serial_next = true;
        }
        else if (this->_serial_queue.empty() == false)
        {
            continue_serial = true;
        }
        else if (this->_finished == false)
        {
            this->_finished = true;
            finish          = true;
        }
    }

    if (parallel_done == false)
    {
        this->try_schedule_parallel();
        return;
    }

    if (start_serial_next || continue_serial)
    {
        this->start_serial_phase();
        return;
    }

    if (finish)
    {
        this->print_final_test_results();
        this->container.exit();
    }
}

void game_bot_controller::start_serial_phase()
{
    bot_integration_test* test   = nullptr;
    bool                  finish = false;
    {
        auto lock                   = std::lock_guard(this->_schedule_mutex);
        this->_serial_phase_started = true;
        if (this->_serial_queue.empty())
        {
            if (this->_finished == false && this->_active_seats.empty() && this->_parallel_queue.empty())
            {
                this->_finished = true;
                finish          = true;
            }
        }
        else
        {
            test = this->_serial_queue.front();
        }
    }

    if (finish)
    {
        this->print_final_test_results();
        this->container.exit();
        return;
    }

    if (test == nullptr)
        return;

    test->suite_slot(1);
    test->extra_slot(std::nullopt);
    fb::logger::info("Starting serial phase with '{}'", test->name());
    this->detach_run_one(test);
}

void game_bot_controller::finish_suite_if_done()
{
    bool finish = false;
    {
        auto lock = std::lock_guard(this->_schedule_mutex);
        if (this->_finished)
            return;
        if (this->_active_seats.empty() == false)
            return;
        if (this->_parallel_queue.empty() == false)
            return;
        if (this->_serial_queue.empty() == false)
            return;

        this->_finished = true;
        finish          = true;
    }

    if (finish)
    {
        this->print_final_test_results();
        this->container.exit();
    }
}

void game_bot_controller::notify_test_ready(bot_integration_test* test)
{
    if (test == nullptr)
        return;
    // Ready wait is signaled from bot_integration_test::notify_ready via promise.
    std::ignore = test;
}

async::task<void> game_bot_controller::on_timer()
{
    co_return;
}

async::task<void> game_bot_controller::on_time(game_bot& bot, const game_resp::time& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_state(game_bot& bot, const game_resp::update_internal_v550& response)
{
    if (ENUM_IN(response.level, UPDATE_STATE_LEVEL::BASED))
    {
        bot.set_nation(response.ch_nation);
        bot.set_divine_beast(response.ch_divine_beast);
        bot.set_level(response.ch_level);
        bot.set_base_hp(response.ch_base_hp);
        bot.set_base_mp(response.ch_base_mp);
        bot.set_strength(response.ch_strength);
        bot.set_intelligence(response.ch_intelligence);
        bot.set_dexterity(response.ch_dexterity);
    }

    if (ENUM_IN(response.level, UPDATE_STATE_LEVEL::HP_MP))
    {
        bot.set_hp(response.ch_hp);
        bot.set_mp(response.ch_mp);
    }

    if (ENUM_IN(response.level, UPDATE_STATE_LEVEL::EXP_MONEY))
    {
        bot.set_exp(response.ch_exp);
        bot.set_money(response.ch_money);
    }

    if (ENUM_IN(response.level, UPDATE_STATE_LEVEL::CROWD_CONTROL))
    {
        bot.set_crowd_control(response.ch_crowd_control);
    }

    bot.set_mail_count(response.ch_mail);
    bot.set_fast_move(response.ch_fast_move);
    co_return;
}

async::task<void> game_bot_controller::on_message(game_bot& bot, const game_resp::message& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_sequence(game_bot& bot, const game_resp::id& response)
{
    bot.set_oid(response.oid);

    if (auto* test = this->owner_of(bot.id); test != nullptr)
        test->try_notify_ready();

    co_return;
}

async::task<void> game_bot_controller::on_position(game_bot& bot, const game_resp::position& response)
{
    bot.set_position(response.abs);
    co_return;
}

async::task<void> game_bot_controller::on_move(game_bot& bot, const game_resp::move& response)
{
    if (bot.oid() != response.id)
        co_return;

    bot.set_position(response.position);
    co_return;
}

async::task<void> game_bot_controller::on_map(game_bot& bot, const game_resp::map_config_v550& response)
{
    bot.set_map(response.id);
    co_return;
}

async::task<void> game_bot_controller::on_transfer(game_bot& bot, const fb::protocol::response::transfer& response)
{
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);

    fb::logger::debug("bot transfer protocol [integration]: bot={} bot_id={} endpoint={}:{} param_bytes={}",
                      bot.name(),
                      bot.id,
                      ip.to_string(),
                      response.port,
                      response.parameter.size());

    bot.close();

    auto created = this->create(response.parameter);
    created->set_transfer_from_bot_id(bot.id);
    created->set_name(bot.name());
    this->reown(bot.id, created->id);
    fb::logger::debug("bot transfer reconnect [integration]: bot={} old_bot_id={} new_bot_id={} endpoint={}:{}",
                      created->name(),
                      bot.id,
                      created->id,
                      ip.to_string(),
                      response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    auto* test = this->owner_of(bot.id);
    if (test == nullptr && bot.transfer_from_bot_id() != 0)
    {
        test = this->owner_of(bot.transfer_from_bot_id());
        if (test != nullptr)
            this->reown(bot.transfer_from_bot_id(), bot.id);
    }

    if (test != nullptr)
    {
        std::shared_ptr<game_bot> bot_shared;
        {
            auto guard = this->_bots.enter_read();
            auto it    = guard.value().find(bot.id);
            bot_shared = (it != guard.value().end()) ? it->second : nullptr;
        }

        if (bot_shared)
            test->on_bot_connected(bot_shared);
    }

    // Do not complete transfer here — TCP connect is not enough.
    // Wait until login finishes (oid/inited) via show/update_external hooks.
    fb::logger::debug("bot transfer login send: bot={} bot_id={} transfer_buffer_bytes={} inited={}",
                      bot.name(),
                      bot.id,
                      bot.transfer_buffer().size(),
                      bot.inited());

    using login_request = fb::protocol::game::request::login<fb::protocol::CLIENT_VERSION::v550>;
    bot.send(login_request(bot.transfer_buffer()), false, true);

    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    fb::logger::debug("Bot {} disconnected from integration testing (bot_id={} inited={})",
                      bot.name(),
                      bot.id,
                      bot.inited());
    co_return;
}

async::task<void> game_bot_controller::on_integration_hook_execution(uint8_t                     opcode,
                                                                     game_bot&                   bot,
                                                                     const fb::protocol::header& header)
{
    auto* test = this->owner_of(bot.id);
    if (test == nullptr)
        co_return;

    auto shared_lock = std::shared_lock<std::shared_mutex>(this->_hook_mutex);
    auto test_it     = this->_test_hooks.find(test);
    if (test_it == this->_test_hooks.end())
        co_return;

    auto cmd_it = test_it->second.find(opcode);
    if (cmd_it == test_it->second.end())
        co_return;

    for (auto& hook : cmd_it->second)
        co_await hook(bot, header);
}

void game_bot_controller::hook_opcode(bot_integration_test* test, uint8_t opcode, hook_function fn)
{
    auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);
    this->_test_hooks[test][opcode].push_back(std::move(fn));
}

void game_bot_controller::unhook_opcode(bot_integration_test* test, uint8_t opcode)
{
    auto unique_lock = std::unique_lock<std::shared_mutex>(this->_hook_mutex);

    auto test_it = this->_test_hooks.find(test);
    if (test_it == this->_test_hooks.end())
        return;

    test_it->second.erase(opcode);
    if (test_it->second.empty())
        this->_test_hooks.erase(test_it);
}

bool game_bot_controller::has_more_tests() const
{
    return this->_parallel_queue.empty() == false || this->_serial_queue.empty() == false ||
           this->_active_seats.empty() == false;
}

void game_bot_controller::print_final_test_results()
{
    fb::logger::info(fb::console::color::cyan, "=== INTEGRATION TEST RESULTS ===");

    int total_tests  = 0;
    int passed_tests = 0;
    int failed_tests = 0;

    {
        auto lock   = std::lock_guard(this->_results_mutex);
        total_tests = static_cast<int>(this->_test_results.size());
        for (const auto& result : this->_test_results)
        {
            if (result.success)
            {
                fb::logger::info(fb::console::color::light_green, "[PASS] {}: {}", result.name, result.message);
                passed_tests++;
            }
            else
            {
                fb::logger::fatal(fb::console::color::light_red, "[FAIL] {}: {}", result.name, result.message);
                failed_tests++;
            }
        }
    }

    fb::logger::info(fb::console::color::cyan, "=== SUMMARY ===");
    fb::logger::info(fb::console::color::light_blue, "Total tests: {}", total_tests);
    fb::logger::info(fb::console::color::light_green, "Passed: {}", passed_tests);
    fb::logger::info(fb::console::color::light_red, "Failed: {}", failed_tests);

    auto elapsed = std::chrono::steady_clock::now() - this->_suite_start;
    auto ms      = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    if (ms < 0)
        ms = 0;
    auto total_sec = ms / 1000;
    auto rem_ms    = static_cast<int>(ms % 1000);
    auto minutes   = total_sec / 60;
    auto seconds   = static_cast<int>(total_sec % 60);
    if (minutes > 0)
    {
        fb::logger::info(fb::console::color::light_blue, "Total elapsed: {}m {:02d}.{:03d}s", minutes, seconds, rem_ms);
    }
    else
    {
        fb::logger::info(fb::console::color::light_blue, "Total elapsed: {}.{:03d}s", seconds, rem_ms);
    }

    if (failed_tests == 0)
    {
        fb::logger::info(fb::console::color::light_green,
                         "ALL TESTS PASSED! Integration test suite completed successfully.");
    }
    else
    {
        fb::logger::fatal(fb::console::color::light_red,
                          "{} TEST(S) FAILED! Integration test suite has failures.",
                          failed_tests);
    }

    fb::logger::info(fb::console::color::cyan, "=== END OF INTEGRATION TEST RESULTS ===");
}
