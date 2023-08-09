#include <fb/bot/bot.h>

using namespace fb::bot;

login_bot::login_bot(bot_container& owner, uint32_t id) : base_bot(owner, id)
{
    this->bind<fb::protocol::login::response::agreement>(std::bind(&login_bot::handle_agreement, this, std::placeholders::_1));
    this->bind<fb::protocol::login::response::message>(std::bind(&login_bot::handle_message, this, std::placeholders::_1));
    this->bind<fb::protocol::response::transfer>(std::bind(&login_bot::handle_transfer, this, std::placeholders::_1));
}

login_bot::login_bot(bot_container& owner, uint32_t id, const fb::buffer& params) : login_bot(owner, id)
{
    auto in_stream = fb::istream((const uint8_t*)params.data(), params.size());
    auto enc_type = in_stream.read_8();
    auto key_size = in_stream.read_8();
    auto enc_key  = new uint8_t[key_size];
    in_stream.read(enc_key, key_size);
    this->_cryptor = fb::cryptor(enc_type, enc_key);
    delete[] enc_key;
}

login_bot::~login_bot()
{}

void login_bot::on_connected()
{
    this->send(fb::protocol::login::request::agreement(this->_cryptor.type(), this->_cryptor.KEY_SIZE, this->_cryptor.key()), false, true);
}

bool login_bot::is_decrypt(int cmd) const
{
    switch(cmd)
    {
        default:
        return base_bot::is_decrypt(cmd);
    }
}

fb::task login_bot::co_login(std::string id, std::string pw)
{
    try
    {
        auto response1 = co_await this->request<fb::protocol::login::response::message>(fb::protocol::login::request::account::create(id, pw));
        if (response1.text.empty() == false)
            throw std::exception("request error");

        std::random_device              rd;
        std::mt19937                    gen(rd());

        uint8_t hair = std::uniform_int_distribution<>(0, 0xFF)(gen);
        uint8_t sex = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t nation = std::uniform_int_distribution<>(0, 1)(gen);
        uint8_t creature = std::uniform_int_distribution<>(0, 3)(gen);
        co_await this->request<fb::protocol::login::response::message>(fb::protocol::login::request::account::complete{ hair, sex, nation, creature});

        this->_try_login = true;
        this->_id = id;
        this->_pw = pw;
        this->send(fb::protocol::login::request::login{ id, pw });
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

void login_bot::handle_agreement(const fb::protocol::login::response::agreement& response)
{
    auto id = boost::uuids::to_string(boost::uuids::random_generator()());
    auto pw = "admin123";
    this->co_login(id, pw);
}

void login_bot::handle_message(const fb::protocol::login::response::message& response)
{ 
    if (this->_try_login)
    {
        if (response.text.empty() == false)
        {
            std::this_thread::sleep_for(1000ms);
            this->send(fb::protocol::login::request::login{ this->_id, this->_pw });
        }
    }
}

void login_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    this->close();
    
    auto bot = this->_owner.create<game_bot>(response.parameter);
    auto ip = boost::asio::ip::address_v4(_byteswap_ulong(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
}