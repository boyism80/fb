#include <fb/bot/bot.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner) : base_bot(owner)
{
    this->bind<fb::protocol::game::response::spell::update>(std::bind(&game_bot::handle_spell_update, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::init>(std::bind(&game_bot::handle_init, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::time>(std::bind(&game_bot::handle_time, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::state>(std::bind(&game_bot::handle_state, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::option>(std::bind(&game_bot::handle_option, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::message>(std::bind(&game_bot::handle_message, this, std::placeholders::_1));
}

game_bot::game_bot(bot_container& owner, const fb::buffer& params) : game_bot(owner)
{
	auto in_stream = fb::istream((const uint8_t*)params.data(), params.size());
    auto enc_type = in_stream.read_8();
    auto key_size = in_stream.read_8();
    auto enc_key  = new uint8_t[key_size];
    in_stream.read(enc_key, key_size);
    this->_cryptor = fb::cryptor(enc_type, enc_key);
    delete[] enc_key;

    this->_transfer_buffer = params;
}

game_bot::~game_bot()
{}

void game_bot::on_connected()
{
    this->send(fb::protocol::game::request::login(this->_transfer_buffer), false, true);
}

void game_bot::handle_spell_update(const fb::protocol::game::response::spell::update& response)
{
    
}

void game_bot::handle_init(const fb::protocol::game::response::init& response)
{
    
}

void game_bot::handle_time(const fb::protocol::game::response::time& response)
{
    
}

void game_bot::handle_state(const fb::protocol::game::response::session::state& response)
{
    
}

void game_bot::handle_option(const fb::protocol::game::response::session::option& response)
{
    
}

void game_bot::handle_message(const fb::protocol::game::response::message& response)
{
    
}
