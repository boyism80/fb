#include <fb/bot/bot.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner, uint32_t id) : base_bot(owner, id)
{
    this->_next_action_time = fb::thread::now();

    this->bind<fb::protocol::game::response::session::id>       (std::bind(&game_bot::handle_sequence,      this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::spell::update>     (std::bind(&game_bot::handle_spell_update,  this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::init>              (std::bind(&game_bot::handle_init,          this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::time>              (std::bind(&game_bot::handle_time,          this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::state>    (std::bind(&game_bot::handle_state,         this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::option>   (std::bind(&game_bot::handle_option,        this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::message>           (std::bind(&game_bot::handle_message,       this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::chat>              (std::bind(&game_bot::handle_chat,          this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::life::action>      (std::bind(&game_bot::handle_action,        this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::object::direction> (std::bind(&game_bot::handle_direction,     this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::position> (std::bind(&game_bot::handle_position,      this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::object::move>      (std::bind(&game_bot::handle_move,          this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::map::config>       (std::bind(&game_bot::handle_map,           this, std::placeholders::_1));
    this->bind<fb::protocol::response::transfer>                (std::bind(&game_bot::handle_transfer,      this, std::placeholders::_1));

    this->pattern(std::bind(&game_bot::pattern_chat,        this), 500ms, 1000ms);
    this->pattern(std::bind(&game_bot::pattern_attack,      this), 500ms, 1000ms);
    this->pattern(std::bind(&game_bot::pattern_direction,   this), 500ms, 1000ms);
    this->pattern(std::bind(&game_bot::pattern_move,        this), 200ms,  500ms);
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

void game_bot::handle_sequence(const fb::protocol::game::response::session::id& response)
{
    this->_sequence = response.sequence;
    std::cout << "sequence : " << this->_sequence << std::endl;
}

void game_bot::handle_spell_update(const fb::protocol::game::response::spell::update& response)
{ }

void game_bot::handle_init(const fb::protocol::game::response::init& response)
{ }

void game_bot::handle_time(const fb::protocol::game::response::time& response)
{ }

void game_bot::handle_state(const fb::protocol::game::response::session::state& response)
{ }

void game_bot::handle_option(const fb::protocol::game::response::session::option& response)
{ }

void game_bot::handle_message(const fb::protocol::game::response::message& response)
{ }

void game_bot::handle_chat(const fb::protocol::game::response::chat& response)
{ }

void game_bot::handle_action(const fb::protocol::game::response::life::action& response)
{ }

void game_bot::handle_direction(const fb::protocol::game::response::object::direction& response)
{ }

void game_bot::handle_position(const fb::protocol::game::response::session::position& response)
{
    this->_position = response.abs;
}

void game_bot::handle_move(const fb::protocol::game::response::object::move& response)
{
    if (this->_sequence != response.id)
        return;

    this->_position = response.position;
}

void game_bot::handle_map(const fb::protocol::game::response::map::config& response)
{ }

void game_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    this->close();

    auto bot = this->_owner.create<game_bot>(response.parameter);
    auto ip = boost::asio::ip::address_v4(_byteswap_ulong(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
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

void game_bot::pattern_direction()
{
    static std::vector<fb::game::direction> directions{ fb::game::direction::LEFT, fb::game::direction::TOP, fb::game::direction::RIGHT, fb::game::direction::BOTTOM };
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::direction{direction});
}

void game_bot::pattern_move()
{
    static std::vector<fb::game::direction> directions{ fb::game::direction::LEFT, fb::game::direction::TOP, fb::game::direction::RIGHT, fb::game::direction::BOTTOM };
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::move{direction, this->_sequence, this->_position });

    switch (direction)
    {
    case fb::game::direction::LEFT:
        this->_position.x--;
        break;

    case fb::game::direction::TOP:
        this->_position.y--;
        break;

    case fb::game::direction::RIGHT:
        this->_position.x++;
        break;

    case fb::game::direction::BOTTOM:
        this->_position.y++;
        break;
    }
}