#include <fb/bot/bot.game.controller.h>
#include <fb/bot/bot.game.h>

using namespace fb::bot;

game_bot_controller::game_bot_controller(bot_container& container) :
    bot_controller<game_bot>(container)
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

    this->_bots.read<void>([&](const auto& bots) {
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

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    // Bot is now managed by controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    // Bot is automatically removed from controller's thread-safe collection
    co_return;
}

bool game_bot_controller::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case fb::protocol::response::transfer::header: // Host discovery
        return false;

    default:
        return true;
    }
}