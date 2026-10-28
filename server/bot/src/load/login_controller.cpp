#include <fb/bot/load/login_controller.h>

#include <fb/bot/container.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/login_bot.h>
#include <fb/config.h>
#include <fb/encoding.h>

#include <cstdint>
#include <exception>
#include <iostream>
#include <random>
#include <string>

using namespace fb::bot::load;

login_bot_controller::login_bot_controller(bot_container& container) :
    fb::bot::login_bot_controller(container),
    _next_account(fb::config<uint32_t>("load:process", 0) * 1000000 + fb::config<uint32_t>("load:account_offset", 0))
{
    for (wchar_t syllable = 0xAC00; syllable <= 0xD7A3; syllable++)
    {
        if (fb::assert_korean(fb::CP949(fb::M(std::wstring(1, syllable)))))
            this->_syllables.push_back(syllable);
    }

    this->bind(&login_bot_controller::on_agreement);
    this->bind(&login_bot_controller::on_transfer);
}

void login_bot_controller::initialize()
{
    // No timers needed for login bot_controller as it's reactive
}

async::task<void> login_bot_controller::on_agreement(login_bot& bot, const login_resp::terms_agreement& response)
{
    auto name = [this](uint32_t index) {
        auto base = static_cast<uint32_t>(this->_syllables.size());
        auto wcs  = std::wstring{};
        for (int i = 0; i < 3; i++)
        {
            wcs.insert(wcs.begin(), this->_syllables.at(index % base));
            index /= base;
        }
        return fb::M(wcs);
    };

    constexpr auto EXISTING_LOGIN_ATTEMPTS = 5;

    auto pw = fb::config<std::string>("load:password", "admin123");

    try
    {
        auto thread  = bot.thread();
        auto entered = false;
        while (entered == false)
        {
            auto id     = name(this->_next_account++);
            auto exists = false;
            while (true)
            {
                auto&& resp = co_await bot.request<login_resp::message>(*bot.make<login_reqs::create>(id, pw));

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
                        id = name(this->_next_account++);
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
                        *bot.make<login_reqs::complete>(hair, gender, nation, divine_beast));

                    if (resp.type == 0x00)
                        break;

                    co_await thread->sleep(100ms);
                }
            }

            for (auto attempt = 0; entered == false && (exists == false || attempt < EXISTING_LOGIN_ATTEMPTS);
                 attempt++)
            {
                auto&& resp = co_await bot.request<login_resp::message>(*bot.make<login_reqs::login>(id, pw));
                if (resp.type == 0x00)
                    entered = true;
                else
                    co_await thread->sleep(1000ms);
            }
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

    auto created = this->container.game->create(response.parameter);
    created->carry_over(bot);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    auto& encryption = bot.encryption();
    bot.send(*bot.make<login_reqs::agreement>(encryption.pattern(),
                                              fb::encryption::KEY_SIZE,
                                              encryption.iv(),
                                              bot.transfer_from(),
                                              bot.client_version(),
                                              bot.ui_mode()),
             false,
             true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> login_bot_controller::on_bot_disconnected(login_bot& bot)
{
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}