#include <fb/bot/bot.login.h>
#include <fb/bot/bot.game.h>
#include <fb/bot/bot.container.h>

using namespace fb::bot;

login_bot::login_bot(bot_container& owner, uint32_t id) :
    base_bot(owner, id)
{
    this->bind(&login_bot::handle_agreement);
    this->bind(&login_bot::handle_transfer);
}

login_bot::login_bot(bot_container& owner, uint32_t id, const fb::stream& params) :
    login_bot(owner, id)
{
    auto clone    = fb::stream{params};
    auto reader   = fb::stream_reader<>(clone);
    auto enc_type = reader.read<uint8_t>();
    auto key_size = reader.read<uint8_t>();
    auto enc_key  = new uint8_t[key_size];
    reader.read(enc_key, key_size);
    this->_crypto = fb::crypto(enc_type, enc_key);
    delete[] enc_key;
}

login_bot::~login_bot()
{ }

std::string fb::bot::login_bot::generate_id() const
{
    constexpr auto min = 0xAC00; // °¡
    constexpr auto max = 0xD7A3; // ÆR

    auto length = random(2, 6);
    auto result = std::wstring{};
    for (int i = 0; i < length; i++)
    {
        result += (wchar_t)random(min, max);
    }

    return fb::M(result);
}

async::task<void> login_bot::on_connected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count++;
    }

    co_await base_bot::on_connected();
    this->send(
        fb::protocol::login::request::agreement(this->_crypto.type(), this->_crypto.KEY_SIZE, this->_crypto.key()),
        false,
        true);
}

async::task<void> login_bot::on_disconnected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count--;
    }
    co_await base_bot::on_disconnected();
}

bool login_bot::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    default:
        return base_bot::decrypt_policy(cmd);
    }
}

async::task<void> login_bot::handle_agreement(const fb::protocol::login::response::agreement& response)
{
    auto id = this->generate_id();
    auto pw = "admin123";

    try
    {
        auto thread = this->thread();

        while (true)
        {
            auto&& resp = co_await this->request<fb::protocol::login::response::message>(
                fb::protocol::login::request::create(id, pw));

            if (resp.type == 0x00)
                break;

            co_await thread->sleep(100ms);
        }

        std::random_device rd;
        std::mt19937       gen(rd());

        uint8_t hair     = std::uniform_int_distribution<>(0, 0xFF)(gen);
        uint8_t sex      = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t nation   = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t creature = std::uniform_int_distribution<>(0, 3)(gen);

        while (true)
        {
            auto&& resp = co_await this->request<fb::protocol::login::response::message>(
                fb::protocol::login::request::complete{hair, sex, nation, creature});

            if (resp.type == 0x00)
                break;

            co_await thread->sleep(100ms);
        }

        while (true)
        {
            auto&& resp = co_await this->request<fb::protocol::login::response::message>(
                fb::protocol::login::request::login{id, pw});
            if (resp.type == 0x00)
                break;

            co_await thread->sleep(1000ms);
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

async::task<void> login_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    this->close();

    auto bot      = this->_owner.create<game_bot>(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
    co_return;
}