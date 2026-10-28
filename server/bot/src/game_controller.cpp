#include <fb/bot/game_controller.h>

#include <fb/bot/container.h>
#include <fb/bot/integration/bulletin_bot.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/bot/integration/protocol_registry.h>
#include <fb/bot/integration/trade_bot.h>
#include <fb/logger.h>

#include <chrono>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <stdexcept>

using namespace fb::bot;

fb::model::item* game_bot_controller::build_item(const Json::Value& json)
{
    auto type = fb::model::build<ITEM_TYPE>(json["type"]);
    switch (type)
    {
    case ITEM_TYPE::STUFF:
        return fb::model::build<fb::model::item*>(json);
    case ITEM_TYPE::CASH:
        return fb::model::build<fb::model::cash*>(json);
    case ITEM_TYPE::CONSUME:
        return fb::model::build<fb::model::consume*>(json);
    case ITEM_TYPE::WEAPON:
        return fb::model::build<fb::model::weapon*>(json);
    case ITEM_TYPE::ARMOR:
        return fb::model::build<fb::model::armor*>(json);
    case ITEM_TYPE::HELMET:
        return fb::model::build<fb::model::helmet*>(json);
    case ITEM_TYPE::RING:
        return fb::model::build<fb::model::ring*>(json);
    case ITEM_TYPE::SHIELD:
        return fb::model::build<fb::model::shield*>(json);
    case ITEM_TYPE::AUXILIARY:
        return fb::model::build<fb::model::auxiliary*>(json);
    case ITEM_TYPE::PACKAGE:
        return fb::model::build<fb::model::pack*>(json);
    default:
        return nullptr;
    }
}

game_bot_controller::game_bot_controller(bot_container& container) :
    bot_controller<game_bot>(container)
{
    this->bind(&game_bot_controller::on_time);
    this->bind<game_resp::map_config>([this](game_bot& bot, auto& response) {
        return this->on_map_config(bot, response);
    });
    this->bind<game_resp::update_internal>([this](game_bot& bot, auto& response) {
        return this->on_state(bot, response);
    });
    this->bind<game_resp::option>([this](game_bot& bot, auto& response) {
        return this->on_option(bot, response);
    });
    this->bind(&game_bot_controller::on_message);
    this->bind(&game_bot_controller::on_sequence);
    this->bind<game_resp::spell_update>([this](game_bot& bot, auto& response) {
        return this->on_spell_update(bot, response);
    });
    this->bind(&game_bot_controller::on_spell_remove);
    this->bind(&game_bot_controller::on_chat);
    this->bind(&game_bot_controller::on_action);
    this->bind(&game_bot_controller::on_direction);
    this->bind(&game_bot_controller::on_position);
    this->bind(&game_bot_controller::on_move);
    this->bind<game_resp::effect>([this](game_bot& bot, auto& response) {
        return this->on_effect(bot, response);
    });
    this->bind(&game_bot_controller::on_hide);
    this->bind(&game_bot_controller::on_die);
    this->bind(&game_bot_controller::on_buff);
    this->bind(&game_bot_controller::on_update_cc);
    this->bind<game_resp::update>([this](game_bot& bot, auto& response) {
        return this->on_update(bot, response);
    });
    this->bind<game_resp::map_config>([this](game_bot& bot, auto& response) {
        return this->on_map(bot, response);
    });
    this->bind(&game_bot_controller::on_transfer);
    this->bind<game_resp::show>([this](game_bot& bot, auto& response) {
        return this->on_show(bot, response);
    });
    this->bind<game_resp::update_external>([this](game_bot& bot, auto& response) {
        return this->on_update_external(bot, response);
    });
    this->bind<game_resp::item_update>([this](game_bot& bot, auto& response) {
        return this->on_item_update(bot, response);
    });
    this->bind(&game_bot_controller::on_item_remove);
    this->bind<game_resp::internal_info>([this](game_bot& bot, auto& response) {
        return this->on_internal_info(bot, response);
    });
    this->bind(&game_bot_controller::on_ping);
    this->bind<game_resp::update_hp>([](game_bot& bot, game_resp::update_hp& response) -> async::task<void> {
        bot.events.push({game_bot::events::TYPE::HIT, response.oid, {}, response.damage, response.percent});
        co_return;
    });
    this->bind<game_resp::user_list>([](game_bot& bot, game_resp::user_list& response) -> async::task<void> {
        bot.record_users(response.users);
        co_return;
    });

    integration::protocol_registry::register_all();
    // Shared opcodes (bulletin 0x31, dialog 0x2F/0x30, trade 0x42) need the
    // unified bot deserializer first. bind_default is first-writer-wins;
    // generated per-subtype types would otherwise own the opcode and later
    // request<bulletin_bot>() would skip binding.
    this->bind_default<integration::bulletin_bot>();
    this->bind<integration::dialog_bot>([](game_bot& bot, integration::dialog_bot& dialog) -> async::task<void> {
        bot.record_dialog(dialog);
        co_return;
    });
    this->bind<integration::dialog_ext_bot>(
        [](game_bot& bot, integration::dialog_ext_bot& dialog) -> async::task<void> {
            bot.record_dialog(dialog);
            co_return;
        });
    this->bind_default<integration::trade_bot>();
    integration::protocol_registry::bind_default(*this);
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    bot.inited(false);
    co_return;
}

async::task<void> game_bot_controller::on_time(game_bot& bot, const game_resp::time& response)
{
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_map_config(game_bot& bot, const game_resp::map_config<V>& response)
{
    bot.set_map(response.id);
    bot.sight.clear();
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_state(game_bot& bot, const game_resp::update_internal<V>& response)
{
    // Update character information based on the state level received
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

    // Always update mail count and fast move setting
    bot.set_mail_count(response.ch_mail);
    bot.set_fast_move(response.ch_fast_move);
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_option(game_bot& bot, const game_resp::option<V>& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_message(game_bot& bot, const game_resp::message& response)
{
    // if (response.text == fb::model::const_value::string::MESSAGE_NOT_READY_GAME_SERVER)
    //{
    //     bot.send(game_reqs::chat(false, "/랜덤이동"));
    // }

    bot.record_message(response.text);
    bot.events.push({game_bot::events::TYPE::MESSAGE, 0, response.text, static_cast<uint32_t>(response.type), 0});

    if (response.type == MESSAGE_TYPE::NOTIFY)
    {
        static const auto regex = boost::xpressive::sregex::compile("(?P<id>.+)> (?P<msg>.+)");
        auto              what  = boost::xpressive::smatch();
        if (boost::xpressive::regex_search(response.text, what, regex) == false)
            co_return;

        static const auto suffix = std::string{"\"에 대한 응답입니다."};
        auto              text   = what["msg"].str();
        if (text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0)
            co_return;

        auto id  = what["id"].str();
        auto msg = std::format("\"{}{}", text, suffix);
        bot.send(*bot.make<game_reqs::whisper>(id, msg));
    }

    co_return;
}

async::task<void> game_bot_controller::on_sequence(game_bot& bot, const game_resp::id& response)
{
    bot.set_oid(response.oid);
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_spell_update(game_bot& bot, const game_resp::spell_update<V>& response)
{
    // Update the bot's spell inventory with the new or updated spell
    bot.update_spell(response.index, response.name, response.type);
    co_return;
}

async::task<void> game_bot_controller::on_spell_remove(game_bot& bot, const game_resp::spell_remove& response)
{
    // Remove the spell from the bot's spell inventory
    bot.remove_spell(response.index);
    co_return;
}

async::task<void> game_bot_controller::on_chat(game_bot& bot, const game_resp::chat& response)
{
    bot.events.push(
        {game_bot::events::TYPE::CHAT, response.oid, response.text, static_cast<uint32_t>(response.type), 0});
    co_return;
}

async::task<void> game_bot_controller::on_action(game_bot& bot, const game_resp::action& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_direction(game_bot& bot, const game_resp::direction& response)
{
    // Update the bot's direction from the server response
    // Only update if the response is for this bot (matching oid)
    if (bot.oid() == response.oid)
    {
        bot.set_direction(response.value);
    }

    co_return;
}

async::task<void> game_bot_controller::on_position(game_bot& bot, const game_resp::position& response)
{
    bot.set_position(response.abs);
    co_return;
}

async::task<void> game_bot_controller::on_move(game_bot& bot, const game_resp::move& response)
{
    if (bot.oid() == response.id)
    {
        bot.set_position(response.position);
    }
    else
    {
        auto position = response.position;
        switch (response.direction)
        {
        case DIRECTION::LEFT:
            position.x--;
            break;

        case DIRECTION::TOP:
            position.y--;
            break;

        case DIRECTION::RIGHT:
            position.x++;
            break;

        case DIRECTION::BOTTOM:
            position.y++;
            break;
        }
        bot.sight.move(response.id, position);
    }
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_map(game_bot& bot, const game_resp::map_config<V>& response)
{
    if (response.id == 1)
    {
        // bot.send(game_reqs::chat(false, "/랜덤이동"));
    }
    else
    {
    }
    co_return;
}

async::task<void> game_bot_controller::on_transfer(game_bot& bot, const fb::protocol::response::transfer& response)
{
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);

    fb::logger::debug("bot transfer protocol: bot={} bot_id={} endpoint={}:{} param_bytes={} pending_contexts={}",
                      bot.name(),
                      bot.id,
                      ip.to_string(),
                      response.port,
                      response.parameter.size(),
                      this->_transfer_contexts.size());

    bot.close();

    auto created = this->create(response.parameter);
    created->carry_over(bot);
    created->set_transfer_from_bot_id(bot.id);
    created->set_name(bot.name());
    fb::logger::debug("bot transfer reconnect: bot={} old_bot_id={} new_bot_id={} endpoint={}:{}",
                      created->name(),
                      bot.id,
                      created->id,
                      ip.to_string(),
                      response.port);
    created->connect(endpoint);

    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_effect(game_bot& bot, const game_resp::effect<V>& response)
{
    // Effect is a one-time event, no need to store state
    co_return;
}

async::task<void> game_bot_controller::on_hide(game_bot& bot, const game_resp::hide& response)
{
    bot.sight.hide(response.oid);
    co_return;
}

async::task<void> game_bot_controller::on_die(game_bot& bot, const game_resp::die& response)
{
    // Update the bot's death state if the response is for this bot
    if (bot.oid() == response.oid)
    {
        bot.set_dead(true);
    }
    else
    {
        bot.sight.hide(response.oid);
    }
    co_return;
}

async::task<void> game_bot_controller::on_buff(game_bot& bot, const game_resp::spell_buff& response)
{
    // spell_unbuff shares opcode 0x3A with spell_buff and is sent as the buff name with a zero duration.
    if (response.duration.total_milliseconds() == 0)
        bot.remove_buff(response.name);
    else
        bot.add_buff(response.name);
    co_return;
}

async::task<void> game_bot_controller::on_update_cc(game_bot& bot, const game_resp::update_cc& response)
{
    bot.set_crowd_control(static_cast<uint32_t>(response.cc));
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_update(game_bot& bot, const game_resp::update<V>& response)
{
    for (auto& object : response.objects_data)
    {
        if (object.oid == bot.oid())
            continue;

        bot.sight.show(object.oid, point<uint16_t>(object.x, object.y), object.look);
    }
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_item_update(game_bot& bot, const game_resp::item_update<V>& response)
{
    // Update the bot's inventory with the new or updated item
    bot.update_item(response.index, response.name, response.count);
    co_return;
}

async::task<void> game_bot_controller::on_item_remove(game_bot& bot, const game_resp::item_remove& response)
{
    // Remove the item from the bot's inventory
    bot.remove_item(static_cast<uint8_t>(response.index));
    co_return;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<void> game_bot_controller::on_internal_info(game_bot& bot, const game_resp::internal_info<V>& response)
{
    bot.set_clan_name(response.clan_name);
    bot.set_clan_title(response.clan_title);
    bot.set_title(response.title);
    bot.set_group_info(response.group_info);
    bot.set_group_option(response.group_option);
    bot.set_remained_exp(response.remained_exp);
    bot.set_class_name(response.class_name);
    bot.mark_info();
    co_return;
}

async::task<void> game_bot_controller::on_ping(game_bot& bot, const game_resp::ping& response)
{
    bot.send(*bot.make<game_reqs::pong>(response.value, 0));
    co_return;
}

bool game_bot_controller::register_transfer_context(std::shared_ptr<transfer_context> context)
{
    auto lock          = std::lock_guard(this->_transfer_mutex);
    auto source_bot_id = context->source_bot_id;
    if (this->_transfer_contexts.contains(source_bot_id))
    {
        fb::logger::warn("bot transfer register rejected: source_bot_id={} (duplicate context)", source_bot_id);
        return false;
    }

    this->_transfer_contexts.insert({source_bot_id, context});
    fb::logger::debug("bot transfer register: source_bot_id={} pending_contexts={}",
                      source_bot_id,
                      this->_transfer_contexts.size());
    return true;
}

void game_bot_controller::remove_transfer_context(uint32_t source_bot_id)
{
    auto       lock   = std::lock_guard(this->_transfer_mutex);
    const auto erased = this->_transfer_contexts.erase(source_bot_id);
    if (erased > 0)
    {
        fb::logger::debug("bot transfer remove: source_bot_id={} pending_contexts={}",
                          source_bot_id,
                          this->_transfer_contexts.size());
    }
    else
    {
        fb::logger::debug("bot transfer remove skipped: source_bot_id={} (no pending context)", source_bot_id);
    }
}

bool game_bot_controller::has_transfer_context(uint32_t source_bot_id) const
{
    auto lock = std::lock_guard(this->_transfer_mutex);
    return this->_transfer_contexts.contains(source_bot_id);
}

bool game_bot_controller::invoke_transfer_context(uint32_t source_bot_id, std::shared_ptr<game_bot> bot)
{
    std::shared_ptr<transfer_context> context;
    {
        auto lock = std::lock_guard(this->_transfer_mutex);
        if (this->_transfer_contexts.contains(source_bot_id) == false)
        {
            fb::logger::warn(
                "bot transfer invoke skipped: source_bot_id={} bot_id={} (no pending context, pending_contexts={})",
                source_bot_id,
                bot != nullptr ? bot->id : 0,
                this->_transfer_contexts.size());
            return false;
        }

        fb::logger::debug("bot transfer complete: source_bot_id={} bot_id={} pending_contexts_before={}",
                          source_bot_id,
                          bot != nullptr ? bot->id : 0,
                          this->_transfer_contexts.size());

        context = this->_transfer_contexts[source_bot_id];
        this->_transfer_contexts.erase(source_bot_id);
    }

    context->complete_success(bot);
    return true;
}

async::task<std::shared_ptr<game_bot>> game_bot::wait_reconnect(const fb::model::timespan& timeout)
{
    auto promise = std::make_shared<async::task_completion_source<std::shared_ptr<game_bot>>>();
    auto context = std::make_shared<game_bot_controller::transfer_context>(
        promise,
        this->id,
        this->controller.weak_from_this_as<game_bot_controller>());
    auto& controller = static_cast<game_bot_controller&>(this->controller);
    if (controller.register_transfer_context(context) == false)
    {
        fb::logger::warn("bot reconnect wait aborted: bot={} (failed to register context)", this->name());
        promise->set_exception(std::make_exception_ptr(std::runtime_error("duplicate transfer context")));
        return promise->task();
    }

    fb::logger::debug("bot reconnect wait: bot={} bot_id={} timeout_ms={}",
                      this->name(),
                      this->id,
                      timeout.total_milliseconds());

    // Set up timeout timer if specified
    if (timeout > 0s)
    {
        auto thread  = this->thread();
        auto builder = thread->new_builder<void>();
        builder.func = [context, timeout](auto& thread) -> async::task<void> {
            context->timer = thread.settimer(
                [context](auto& datetime, auto thread_id) -> async::task<void> {
                    context->complete_timeout();
                    co_return;
                },
                timeout,
                fb::timer::repeat_type::once);
            co_return;
        };
        builder.enqueue();
    }

    return promise->task();
}

async::task<std::shared_ptr<game_bot>>
game_bot::transfer(const fb::protocol::header& protocol, const fb::model::timespan& timeout, bool encrypt, bool wrap)
{
    auto task = this->wait_reconnect(timeout);
    this->send(protocol, encrypt, wrap);
    return task;
}

async::task<bool> game_bot::logout(const fb::model::timespan& timeout)
{
    auto& controller = this->controller;
    auto  id         = this->id;
    auto  thread     = this->thread();
    if (controller.contains(id) == false)
        co_return false;

    // A real client logs out by closing the socket; the server saves and releases the session on its own.
    this->close();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout.total_milliseconds());
    while (controller.contains(id))
    {
        if (std::chrono::steady_clock::now() >= deadline)
            co_return false;

        co_await thread->sleep(50ms);
    }
    co_return true;
}
