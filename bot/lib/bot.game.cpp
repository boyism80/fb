#include <fb/bot/bot.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner, uint32_t id) : base_bot(owner, id)
{
    this->_next_action_time = fb::thread::now();

    this->bind<fb::protocol::game::response::spell::update>   (std::bind(&game_bot::handle_spell_update, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::init>            (std::bind(&game_bot::handle_init,         this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::time>            (std::bind(&game_bot::handle_time,         this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::state>  (std::bind(&game_bot::handle_state,        this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::option> (std::bind(&game_bot::handle_option,       this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::message>         (std::bind(&game_bot::handle_message,      this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::chat>            (std::bind(&game_bot::handle_chat,         this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::life::action>    (std::bind(&game_bot::handle_action,       this, std::placeholders::_1));

    this->pattern(std::bind(&game_bot::pattern_chat,   this), 500ms, 1000ms);
    this->pattern(std::bind(&game_bot::pattern_attack, this), 500ms, 1000ms);
}

game_bot::game_bot(bot_container& owner, uint32_t id, const fb::buffer& params) : game_bot(owner, id)
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

void game_bot::pattern(std::function<void()> fn, const std::chrono::steady_clock::duration& min, const std::chrono::steady_clock::duration& max)
{
    this->_pattern_params.push_back(pattern_params { fn, min, max });
}

void game_bot::on_connected()
{
    this->send(fb::protocol::game::request::login(this->_transfer_buffer), false, true);
}

void game_bot::on_timer(std::chrono::steady_clock::duration now)
{
    if (now < this->_next_action_time)
        return;

    static std::random_device device;
    static std::mt19937 gen(device());
    std::uniform_int_distribution<> dist(0, this->_pattern_params.size() - 1);

    auto& pattern = this->_pattern_params.at(dist(gen));
    pattern.fn();

    auto rand_term = std::uniform_int_distribution<long long>(pattern.min.count(), pattern.max.count())(gen);
    this->_next_action_time = now + std::chrono::steady_clock::duration(rand_term);
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

void game_bot::handle_chat(const fb::protocol::game::response::chat& response)
{
    std::cout << "message : " << response.text << std::endl;
}

void game_bot::handle_action(const fb::protocol::game::response::life::action& response)
{
    std::cout << "action : " << (uint8_t)response.value << std::endl;
}

void game_bot::pattern_chat()
{
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::vector<std::string> messages{ "hello world1", "hello world2", "hello world3" };
    static std::uniform_int_distribution<> dist(0, messages.size() - 1);
    
    auto& message = messages.at(dist(gen));
    this->send(fb::protocol::game::request::chat(false, message));
}

void game_bot::pattern_attack()
{
    this->send(fb::protocol::game::request::attack());
}