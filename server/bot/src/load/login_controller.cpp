#include <fb/bot/container.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/load/login_controller.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/login_bot.h>

using namespace fb::bot::load;

namespace {

// create / complete / login C2S layouts carry no version delta, so the bot always
// builds them with the primary (v550) specialization.
constexpr auto REQUEST_VERSION = fb::protocol::CLIENT_VERSION::v550;

} // namespace

login_bot_controller::login_bot_controller(bot_container& container) :
    fb::bot::login_bot_controller(container)
{
    this->bind(&login_bot_controller::on_agreement);
    this->bind(&login_bot_controller::on_transfer);
}

void login_bot_controller::initialize()
{
    // No timers needed for login bot_controller as it's reactive
}

async::task<void> login_bot_controller::on_agreement(login_bot& bot, const login_resp::terms_agreement& response)
{
    auto           id     = bot.generate_id();
    auto           exists = false;
    constexpr auto pw     = "admin123";

    try
    {
        auto thread = bot.thread();

        while (true)
        {
            auto&& resp = co_await bot.request<login_resp::message>(login_reqs::create<REQUEST_VERSION>(id, pw));

            if (resp.type == 0x00)
                break;

            if (resp.type == 0x0E)
            {
                if (resp.text == fb::model::const_value::string::MESSAGE_ACCOUNT_ALREADY_EXISTS)
                {
                    exists = true;
                    break;
                }
                else
                {
                    id = bot.generate_id();
                }
            }

            co_await thread->sleep(100ms);
        }

        if (exists == false)
        {
            std::random_device rd;
            std::mt19937       gen(rd());

            uint8_t hair         = std::uniform_int_distribution<>(0, 101)(gen);
            uint8_t gender       = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t nation       = std::uniform_int_distribution<>(1, 2)(gen);
            uint8_t divine_beast = std::uniform_int_distribution<>(0, 3)(gen);

            while (true)
            {
                auto&& resp = co_await bot.request<login_resp::message>(
                    login_reqs::complete<REQUEST_VERSION>{hair, gender, nation, divine_beast});

                if (resp.type == 0x00)
                    break;

                co_await thread->sleep(100ms);
            }
        }

        while (true)
        {
            auto&& resp = co_await bot.request<login_resp::message>(login_reqs::login<REQUEST_VERSION>{id, pw});
            if (resp.type == 0x00)
                break;

            co_await thread->sleep(1000ms);
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    co_return;
}

async::task<void> login_bot_controller::on_transfer(login_bot& bot, const fb_resp::transfer& response)
{
    bot.close();

    auto created  = this->container.game->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    auto& encryption = bot.encryption();

    // Runtime CLIENT_VERSION -> compile-time V for the versioned request layout.
    fb::protocol::visit_client_version(bot.client_version(), [&]<fb::protocol::CLIENT_VERSION V> {
        bot.send(login_reqs::agreement<V>(encryption.pattern(),
                                          fb::encryption::KEY_SIZE,
                                          encryption.iv(),
                                          bot.transfer_from(),
                                          bot.client_version()),
                 false,
                 true);
    });

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> login_bot_controller::on_bot_disconnected(login_bot& bot)
{
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}