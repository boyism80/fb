#include <fb/bot/bot.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner, uint32_t id) :
    base_bot(owner, id)
{
    this->_next_action_time = fb::model::datetime();

    this->bind<fb::protocol::game::response::session::id>(std::bind(&game_bot::handle_sequence, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::spell::update>(std::bind(&game_bot::handle_spell_update, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::init>(std::bind(&game_bot::handle_init, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::time>(std::bind(&game_bot::handle_time, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::state>(std::bind(&game_bot::handle_state, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::option>(std::bind(&game_bot::handle_option, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::message>(std::bind(&game_bot::handle_message, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::chat>(std::bind(&game_bot::handle_chat, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::life::action>(std::bind(&game_bot::handle_action, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::object::direction>(std::bind(&game_bot::handle_direction, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::session::position>(std::bind(&game_bot::handle_position, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::object::move>(std::bind(&game_bot::handle_move, this, std::placeholders::_1));
    this->bind<fb::protocol::game::response::map::config>(std::bind(&game_bot::handle_map, this, std::placeholders::_1));
    this->bind<fb::protocol::response::transfer>(std::bind(&game_bot::handle_transfer, this, std::placeholders::_1));

    this->pattern(std::bind(&game_bot::pattern_chat, this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_attack,      this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_direction,   this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_move,        this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_pickup,      this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_emotion,     this), 1000ms, 2000ms);
    // this->pattern(std::bind(&game_bot::pattern_board_sections, this), 1000ms, 2000ms);
}

game_bot::game_bot(bot_container& owner, uint32_t id, const fb::buffer& params) :
    game_bot(owner, id)
{
    auto in_stream = fb::istream((const uint8_t*)params.data(), params.size());
    auto enc_type  = in_stream.read_8();
    auto key_size  = in_stream.read_8();
    auto enc_key   = new uint8_t[key_size];
    in_stream.read(enc_key, key_size);
    this->_cryptor = fb::cryptor(enc_type, enc_key);
    delete[] enc_key;

    this->_transfer_buffer = params;
}

game_bot::~game_bot()
{ }

void game_bot::pattern(std::function<void()> fn, const std::chrono::steady_clock::duration& min, const std::chrono::steady_clock::duration& max)
{
    this->_pattern_params.push_back(pattern_params{fn, min, max});
}

void game_bot::on_connected()
{
    this->send(fb::protocol::game::request::login(this->_transfer_buffer), false, true);
}

void game_bot::on_timer(std::chrono::steady_clock::duration now)
{
    if (this->_inited == false)
        return;

    if (now < this->_next_action_time)
        return;

    static std::random_device       device;
    static std::mt19937             gen(device());
    std::uniform_int_distribution<> dist(0, this->_pattern_params.size() - 1);

    auto&                           pattern = this->_pattern_params.at(dist(gen));
    pattern.fn();

    auto rand_term          = std::uniform_int_distribution<long long>(pattern.min.count(), pattern.max.count())(gen);
    this->_next_action_time = now + std::chrono::steady_clock::duration(rand_term);
}

async::task<void> game_bot::handle_sequence(const fb::protocol::game::response::session::id& response)
{
    this->_sequence = response.sequence;
    std::cout << "sequence : " << this->_sequence << std::endl;
    co_return;
}

async::task<void> game_bot::handle_spell_update(const fb::protocol::game::response::spell::update& response)
{
    co_return;
}

async::task<void> game_bot::handle_init(const fb::protocol::game::response::init& response)
{
    this->_inited = true;
    co_return;
}

async::task<void> game_bot::handle_time(const fb::protocol::game::response::time& response)
{
    co_return;
}

async::task<void> game_bot::handle_state(const fb::protocol::game::response::session::state& response)
{
    co_return;
}

async::task<void> game_bot::handle_option(const fb::protocol::game::response::session::option& response)
{
    co_return;
}

async::task<void> game_bot::handle_message(const fb::protocol::game::response::message& response)
{
    co_return;
}

async::task<void> game_bot::handle_chat(const fb::protocol::game::response::chat& response)
{
    co_return;
}

async::task<void> game_bot::handle_action(const fb::protocol::game::response::life::action& response)
{
    co_return;
}

async::task<void> game_bot::handle_direction(const fb::protocol::game::response::object::direction& response)
{
    co_return;
}

async::task<void> game_bot::handle_position(const fb::protocol::game::response::session::position& response)
{
    this->_position = response.abs;
    co_return;
}

async::task<void> game_bot::handle_move(const fb::protocol::game::response::object::move& response)
{
    if (this->_sequence != response.id)
        co_return;

    this->_position = response.position;
}

async::task<void> game_bot::handle_map(const fb::protocol::game::response::map::config& response)
{
    co_return;
}

async::task<void> game_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    this->close();

    auto bot      = this->_owner.create<game_bot>(response.parameter);
    auto ip       = boost::asio::ip::address_v4(_byteswap_ulong(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
    co_return;
}

async::task<void> game_bot::pattern_chat()
{
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::vector<std::string>        messages{"/서버종료"};
    static std::uniform_int_distribution<> dist(0, messages.size() - 1);

    auto&                                  message = messages.at(dist(gen));
    this->send(fb::protocol::game::request::chat(false, message));
    co_return;
}

async::task<void> game_bot::pattern_attack()
{
    this->send(fb::protocol::game::request::attack());
    co_return;
}

async::task<void> game_bot::pattern_direction()
{
    static std::vector<DIRECTION>          directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto                                   direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::direction{direction});
    co_return;
}

async::task<void> game_bot::pattern_move()
{
    static std::vector<DIRECTION>          directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto                                   direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::move{direction, this->_sequence, this->_position});

    switch (direction)
    {
    case DIRECTION::LEFT:
        this->_position.x--;
        break;

    case DIRECTION::TOP:
        this->_position.y--;
        break;

    case DIRECTION::RIGHT:
        this->_position.x++;
        break;

    case DIRECTION::BOTTOM:
        this->_position.y++;
        break;
    }
    co_return;
}

async::task<void> game_bot::pattern_pickup()
{
    this->send(fb::protocol::game::request::pick_up{false});
    co_return;
}

async::task<void> game_bot::pattern_emotion()
{
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::uniform_int_distribution<> dist(0, 0xFF - 0x0B);

    this->send(fb::protocol::game::request::emotion{(uint8_t)dist(gen)});
    co_return;
}

async::task<void> game_bot::pattern_board_sections()
{
    std::random_device              device;
    std::mt19937                    gen(device());
    std::uniform_int_distribution<> dist(0, 1);

    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::SECTIONS));

    auto section = (uint32_t)dist(gen);
    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::ARTICLES, section));
    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::ARTICLE, section, 0));
    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::WRITE, section, 0, 0, "게시글 타이틀", "게시글 내용"));
    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::DELETE, section, 0));
    co_return;
}