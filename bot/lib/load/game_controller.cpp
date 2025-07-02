#include <fb/bot/load/game_controller.h>
#include <fb/bot/game_bot.h>

using namespace fb::bot::load;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{
    this->bind(&game_bot_controller::handle_time);
    this->bind(&game_bot_controller::handle_state);
    this->bind(&game_bot_controller::handle_option);
    this->bind(&game_bot_controller::handle_message);
    this->bind(&game_bot_controller::handle_sequence);
    this->bind(&game_bot_controller::handle_spell_update);
    this->bind(&game_bot_controller::handle_chat);
    this->bind(&game_bot_controller::handle_action);
    this->bind(&game_bot_controller::handle_direction);
    this->bind(&game_bot_controller::handle_position);
    this->bind(&game_bot_controller::handle_move);
    this->bind(&game_bot_controller::handle_sound);
    this->bind(&game_bot_controller::handle_effect);
    this->bind(&game_bot_controller::handle_hide);
    this->bind(&game_bot_controller::handle_die);
    this->bind(&game_bot_controller::handle_buff);
    this->bind(&game_bot_controller::handle_unbuff);
    this->bind(&game_bot_controller::handle_update);
    this->bind(&game_bot_controller::handle_map);
    this->bind(&game_bot_controller::handle_transfer);
}

void game_bot_controller::initialize()
{
    this->bind_thread_timer(&game_bot_controller::handle_timer, 100ms);
}

async::task<void> game_bot_controller::handle_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    this->_bots.read([&](const auto& bots) {
        for (auto& [_, bot] : params->bots)
        {
            if (bots.contains(bot->id) == false)
                continue;

            auto typed_bot = static_cast<game_bot*>(bot.get());
            typed_bot->process_random_pattern(now);
        }
    });
    co_return;
}

async::task<void> game_bot_controller::handle_time(game_bot& bot, const fb::protocol::game::response::time& response)
{
    co_return;
}

async::task<void> game_bot_controller::handle_state(game_bot&                                            bot,
                                                    const fb::protocol::game::response::update_internal& response)
{
    // Update character information based on the state level received
    if (ENUM_IN(response.level, STATE_LEVEL::BASED))
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

    if (ENUM_IN(response.level, STATE_LEVEL::HP_MP))
    {
        bot.set_hp(response.ch_hp);
        bot.set_mp(response.ch_mp);
    }

    if (ENUM_IN(response.level, STATE_LEVEL::EXP_MONEY))
    {
        bot.set_exp(response.ch_exp);
        bot.set_money(response.ch_money);
    }

    if (ENUM_IN(response.level, STATE_LEVEL::CROWD_CONTROL))
    {
        bot.set_crowd_control(response.ch_crowd_control);
    }

    // Always update mail count and fast move setting
    bot.set_mail_count(response.ch_mail);
    bot.set_fast_move(response.ch_fast_move);

    co_return;
}

async::task<void> game_bot_controller::handle_option(game_bot&                                   bot,
                                                     const fb::protocol::game::response::option& response)
{
    co_return;
}

async::task<void> game_bot_controller::handle_message(game_bot&                                    bot,
                                                      const fb::protocol::game::response::message& response)
{
    // if (response.text == fb::model::const_value::string::MESSAGE_NOT_READY_GAME_SERVER)
    //{
    //     bot.send(fb::protocol::game::request::chat(false, "/랜덤이동"));
    // }

    if (response.type == MESSAGE_TYPE::NOTIFY)
    {
        static const auto regex = boost::xpressive::sregex::compile("(?P<id>.+)> (?P<msg>.+)");
        auto              what  = boost::xpressive::smatch();
        if (boost::xpressive::regex_search(response.text, what, regex) == false)
            co_return;

        auto id  = what["id"].str();
        auto msg = std::format("\"{}\"에 대한 응답입니다.", what["msg"].str());
        bot.send(fb::protocol::game::request::whisper(id, msg));
    }

    co_return;
}

async::task<void> game_bot_controller::handle_sequence(game_bot& bot, const fb::protocol::game::response::id& response)
{
    bot.set_sequence(response.sequence);
    co_return;
}

async::task<void> game_bot_controller::handle_spell_update(game_bot&                                         bot,
                                                           const fb::protocol::game::response::spell_update& response)
{
    // Add the spell to the bot's active spells by name
    bot.add_spell(response.name);
    co_return;
}

async::task<void> game_bot_controller::handle_chat(game_bot& bot, const fb::protocol::game::response::chat& response)
{
    co_return;
}

async::task<void> game_bot_controller::handle_action(game_bot&                                   bot,
                                                     const fb::protocol::game::response::action& response)
{
    co_return;
}

async::task<void> game_bot_controller::handle_direction(game_bot&                                      bot,
                                                        const fb::protocol::game::response::direction& response)
{
    // Update the bot's direction from the server response
    // Only update if the response is for this bot (matching sequence)
    if (bot.sequence() == response.sequence)
    {
        bot.set_direction(response.value);
    }

    co_return;
}

async::task<void> game_bot_controller::handle_position(game_bot&                                     bot,
                                                       const fb::protocol::game::response::position& response)
{
    bot.set_position(response.abs);
    co_return;
}

async::task<void> game_bot_controller::handle_move(game_bot& bot, const fb::protocol::game::response::move& response)
{
    if (bot.sequence() != response.id)
        co_return;

    bot.set_position(response.position);
    co_return;
}

async::task<void> game_bot_controller::handle_map(game_bot&                                       bot,
                                                  const fb::protocol::game::response::map_config& response)
{
    if (response.id == 1)
    {
        // bot.send(fb::protocol::game::request::chat(false, "/랜덤이동"));
    }
    else
    {
    }
    bot.set_initialized(true);
    co_return;
}

async::task<void> game_bot_controller::handle_transfer(game_bot& bot, const fb::protocol::response::transfer& response)
{
    bot.close();

    auto created  = this->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> game_bot_controller::handle_sound(game_bot& bot, const fb::protocol::game::response::sound& response)
{
    // Sound is a one-time event, no need to store state
    co_return;
}

async::task<void> game_bot_controller::handle_effect(game_bot&                                   bot,
                                                     const fb::protocol::game::response::effect& response)
{
    // Effect is a one-time event, no need to store state
    co_return;
}

async::task<void> game_bot_controller::handle_hide(game_bot& bot, const fb::protocol::game::response::hide& response)
{
    // Hide events are for other players disappearing, not relevant for bot's own state
    co_return;
}

async::task<void> game_bot_controller::handle_die(game_bot& bot, const fb::protocol::game::response::die& response)
{
    // Update the bot's death state if the response is for this bot
    if (bot.sequence() == response.id)
    {
        bot.set_dead(true);
    }
    co_return;
}

async::task<void> game_bot_controller::handle_buff(game_bot&                                       bot,
                                                   const fb::protocol::game::response::spell_buff& response)
{
    // Add the buff to the bot's active buffs by name
    bot.add_buff(response.name);
    co_return;
}

async::task<void> game_bot_controller::handle_unbuff(game_bot&                                         bot,
                                                     const fb::protocol::game::response::spell_unbuff& response)
{
    // Remove the buff from the bot's active buffs by name
    bot.remove_buff(response.buff_name);
    co_return;
}

async::task<void> game_bot_controller::handle_update(game_bot&                                   bot,
                                                     const fb::protocol::game::response::update& response)
{
    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}