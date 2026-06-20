#include <fb/bot/game_controller.h>
#include <fb/bot/container.h>
#include <fb/bot/integration/protocol_registry.h>

using namespace fb::bot;

game_bot_controller::game_bot_controller(bot_container& container) :
    bot_controller<game_bot>(container)
{
    this->bind(&game_bot_controller::on_time);
    this->bind(&game_bot_controller::on_map_config);
    this->bind(&game_bot_controller::on_state);
    this->bind(&game_bot_controller::on_option);
    this->bind(&game_bot_controller::on_message);
    this->bind(&game_bot_controller::on_sequence);
    this->bind(&game_bot_controller::on_spell_update);
    this->bind(&game_bot_controller::on_spell_remove);
    this->bind(&game_bot_controller::on_chat);
    this->bind(&game_bot_controller::on_action);
    this->bind(&game_bot_controller::on_direction);
    this->bind(&game_bot_controller::on_position);
    this->bind(&game_bot_controller::on_move);
    this->bind(&game_bot_controller::on_effect);
    this->bind(&game_bot_controller::on_hide);
    this->bind(&game_bot_controller::on_die);
    this->bind(&game_bot_controller::on_buff);
    this->bind(&game_bot_controller::on_unbuff);
    this->bind(&game_bot_controller::on_update);
    this->bind(&game_bot_controller::on_map);
    this->bind(&game_bot_controller::on_transfer);
    this->bind(&game_bot_controller::on_update_external<true>);
    this->bind(&game_bot_controller::on_update_external<false>);
    this->bind(&game_bot_controller::on_item_update);
    this->bind(&game_bot_controller::on_item_remove);
    this->bind(&game_bot_controller::on_internal_info);
    this->bind(&game_bot_controller::on_ping);

    integration::protocol_registry::register_all();
}

bool game_bot_controller::decrypt_policy(int opcode) const
{
    switch (opcode)
    {
    case fb::protocol::response::transfer::opcode: // Host discovery
        return false;

    default:
        return true;
    }
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

async::task<void> game_bot_controller::on_map_config(game_bot& bot, const game_resp::map_config& response)
{
    bot.set_map(response.id);
    co_return;
}

async::task<void> game_bot_controller::on_state(game_bot& bot, const game_resp::update_internal& response)
{
    // Update character information based on the state level received
    if (ENUM_IN(response.level, UPDATE_STATE_LEVEL::BASED))
    {
        bot.set_nation(response.ch_nation);
        bot.set_creature(response.ch_creature);
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

async::task<void> game_bot_controller::on_option(game_bot& bot, const game_resp::option& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_message(game_bot& bot, const game_resp::message& response)
{
    // if (response.text == fb::model::const_value::string::MESSAGE_NOT_READY_GAME_SERVER)
    //{
    //     bot.send(game_reqs::chat(false, "/랜덤이동"));
    // }

    if (response.type == MESSAGE_TYPE::NOTIFY)
    {
        static const auto regex = boost::xpressive::sregex::compile("(?P<id>.+)> (?P<msg>.+)");
        auto              what  = boost::xpressive::smatch();
        if (boost::xpressive::regex_search(response.text, what, regex) == false)
            co_return;

        auto id  = what["id"].str();
        auto msg = std::format("\"{}\"에 대한 응답입니다.", what["msg"].str());
        bot.send(game_reqs::whisper(id, msg));
    }

    co_return;
}

async::task<void> game_bot_controller::on_sequence(game_bot& bot, const game_resp::id& response)
{
    bot.set_oid(response.oid);
    co_return;
}

async::task<void> game_bot_controller::on_spell_update(game_bot& bot, const game_resp::spell_update& response)
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
    if (bot.oid() != response.id)
        co_return;

    bot.set_position(response.position);
    co_return;
}

async::task<void> game_bot_controller::on_map(game_bot& bot, const game_resp::map_config& response)
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
    bot.close();

    auto created  = this->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> game_bot_controller::on_effect(game_bot& bot, const game_resp::effect& response)
{
    // Effect is a one-time event, no need to store state
    co_return;
}

async::task<void> game_bot_controller::on_hide(game_bot& bot, const game_resp::hide& response)
{
    // Hide events are for other players disappearing, not relevant for bot's own state
    co_return;
}

async::task<void> game_bot_controller::on_die(game_bot& bot, const game_resp::die& response)
{
    // Update the bot's death state if the response is for this bot
    if (bot.oid() == response.oid)
    {
        bot.set_dead(true);
    }
    co_return;
}

async::task<void> game_bot_controller::on_buff(game_bot& bot, const game_resp::spell_buff& response)
{
    // Add the buff to the bot's active buffs by name
    bot.add_buff(response.name);
    co_return;
}

async::task<void> game_bot_controller::on_unbuff(game_bot& bot, const game_resp::spell_unbuff& response)
{
    // Remove the buff from the bot's active buffs by name
    bot.remove_buff(response.buff_name);
    co_return;
}

async::task<void> game_bot_controller::on_update(game_bot& bot, const game_resp::update& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_item_update(game_bot& bot, const game_resp::item_update& response)
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

async::task<void> game_bot_controller::on_internal_info(game_bot& bot, const game_resp::internal_info& response)
{
    bot.set_clan_name(response.clan_name);
    bot.set_clan_title(response.clan_title);
    bot.set_title(response.title);
    bot.set_group_info(response.group_info);
    bot.set_group_option(response.group_option);
    bot.set_remained_exp(response.remained_exp);
    co_return;
}

async::task<void> game_bot_controller::on_ping(game_bot& bot, const game_resp::ping& response)
{
    bot.send(game_reqs::pong(response.value, 0));
    co_return;
}

bool game_bot_controller::register_transfer_context(const fb::protocol::header&       protocol,
                                                    std::shared_ptr<transfer_context> context)
{
    auto name = context->name;
    if (this->_transfer_contexts.contains(name))
        return false;

    this->_transfer_contexts.insert({name, context});
    return true;
}

void game_bot_controller::remove_transfer_context(std::string name)
{
    this->_transfer_contexts.erase(name);
}

bool game_bot_controller::invoke_transfer_context(std::string name, std::shared_ptr<game_bot> bot)
{
    if (this->_transfer_contexts.contains(name) == false)
        return false;

    auto context = this->_transfer_contexts[name];
    this->_transfer_contexts.erase(name);
    context->complete_success(bot);
    return true;
}

async::task<std::shared_ptr<game_bot>>
game_bot::transfer(const fb::protocol::header& protocol, const fb::model::timespan& timeout, bool encrypt, bool wrap)
{
    auto promise = std::make_shared<async::task_completion_source<std::shared_ptr<game_bot>>>();
    auto context = std::make_shared<game_bot_controller::transfer_context>(
        promise,
        this->name(),
        this->controller.weak_from_this_as<game_bot_controller>());
    auto& controller = static_cast<game_bot_controller&>(this->controller);
    controller.register_transfer_context(protocol, context);

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

    this->send(protocol, encrypt, wrap);
    return promise->task();
}
