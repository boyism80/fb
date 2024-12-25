#include <bot.h>

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
    this->_cryptor = fb::cryptor(enc_type, enc_key);
    delete[] enc_key;
}

login_bot::~login_bot()
{ }

async::task<void> login_bot::on_connected()
{
    co_await base_bot::on_connected();
    fb::logger::info("login bot spawned");
    this->send(
        fb::protocol::login::request::agreement(this->_cryptor.type(), this->_cryptor.KEY_SIZE, this->_cryptor.key()),
        false,
        true);
}

async::task<void> login_bot::on_disconnected()
{
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
    auto id = boost::uuids::to_string(boost::uuids::random_generator()());
    auto pw = "admin123";

    try
    {
        auto&& response1 = co_await this->request<fb::protocol::login::response::message>(
            fb::protocol::login::request::account::create(id, pw));
        if (response1.text.empty() == false)
            throw std::runtime_error("request error");

        std::random_device rd;
        std::mt19937       gen(rd());

        uint8_t hair     = std::uniform_int_distribution<>(0, 0xFF)(gen);
        uint8_t sex      = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t nation   = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t creature = std::uniform_int_distribution<>(0, 3)(gen);
        co_await this->request<fb::protocol::login::response::message>(
            fb::protocol::login::request::account::complete{hair, sex, nation, creature});

        while (true)
        {
            auto&& resp = co_await this->request<fb::protocol::login::response::message>(
                fb::protocol::login::request::login{id, pw});
            if (resp.text.empty())
                break;

            auto thread = this->thread();
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
    co_await bot->connect(endpoint);
}