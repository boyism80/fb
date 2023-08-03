#include <fb/bot/bot.h>

using namespace fb::bot;

login_bot::login_bot(bot_container& owner) : base_bot(owner)
{
    this->bind<fb::protocol::login::response::agreement>(std::bind(&login_bot::handle_agreement, this, std::placeholders::_1));
}

login_bot::login_bot(bot_container& owner, const fb::buffer& params) : login_bot(owner)
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

void login_bot::handle_agreement(const fb::protocol::login::response::agreement& response)
{

}