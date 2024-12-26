#include <bot.game.h>
#include <bot.container.h>

using namespace fb::bot;

game_bot::game_bot(bot_container& owner, uint32_t id) :
    base_bot(owner, id)
{
    this->_next_action_time = fb::model::datetime();

    this->bind(&game_bot::handle_sequence);
    this->bind(&game_bot::handle_spell_update);
    this->bind(&game_bot::handle_time);
    this->bind(&game_bot::handle_state);
    this->bind(&game_bot::handle_option);
    this->bind(&game_bot::handle_message);
    this->bind(&game_bot::handle_chat);
    this->bind(&game_bot::handle_action);
    this->bind(&game_bot::handle_direction);
    this->bind(&game_bot::handle_position);
    this->bind(&game_bot::handle_move);
    this->bind(&game_bot::handle_map);
    this->bind(&game_bot::handle_transfer);

    this->pattern(&game_bot::pattern_chat, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_attack, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_direction, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_move, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_pickup, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_emotion, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_board_sections, 250ms, 1000ms);
}

game_bot::game_bot(bot_container& owner, uint32_t id, const fb::stream& params) :
    game_bot(owner, id)
{
    auto clone    = fb::stream{params};
    auto reader   = fb::stream_reader<>(clone);
    auto enc_type = reader.read<uint8_t>();
    auto key_size = reader.read<uint8_t>();
    auto enc_key  = new uint8_t[key_size];
    reader.read(enc_key, key_size);
    this->_cryptor = fb::cryptor(enc_type, enc_key);
    delete[] enc_key;

    this->_transfer_buffer = params;
}

game_bot::~game_bot()
{ }

async::task<void> game_bot::on_connected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count++;
    }

    co_await base_bot::on_connected();
    auto&& resp = co_await this->request<fb::protocol::game::response::map::config>(
        fb::protocol::game::request::login(this->_transfer_buffer),
        false,
        true);

    if (resp.id == 1)
        this->send(fb::protocol::game::request::chat(false, "/랜덤이동"));
    else
        this->_inited = true;
}

async::task<void> game_bot::on_disconnected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count--;
    }

    co_await base_bot::on_disconnected();
}

async::task<void> game_bot::on_timer(const fb::model::datetime& now)
{
    if (!this->_inited)
        co_return;

    if (now < this->_next_action_time)
        co_return;

    static std::random_device       device;
    static std::mt19937             gen(device());
    std::uniform_int_distribution<> dist(0, this->_pattern_params.size() - 1);

    auto& pattern  = this->_pattern_params.at(dist(gen));
    auto  datetime = fb::model::datetime{now};
    co_await pattern.fn();

    auto rand_term          = std::uniform_int_distribution<long long>(pattern.min.count(), pattern.max.count())(gen);
    this->_next_action_time = datetime + std::chrono::steady_clock::duration(rand_term);
}

async::task<void> game_bot::handle_sequence(const fb::protocol::game::response::character::id& response)
{
    this->_sequence = response.sequence;
    co_return;
}

async::task<void> game_bot::handle_spell_update(const fb::protocol::game::response::spell::update& response)
{
    co_return;
}

async::task<void> game_bot::handle_time(const fb::protocol::game::response::time& response)
{
    co_return;
}

async::task<void> game_bot::handle_state(const fb::protocol::game::response::character::state& response)
{
    co_return;
}

async::task<void> game_bot::handle_option(const fb::protocol::game::response::character::option& response)
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

async::task<void> game_bot::handle_position(const fb::protocol::game::response::character::position& response)
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
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
    co_return;
}

async::task<void> game_bot::pattern_chat()
{
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::vector<std::string>        messages{"안녕하세요", "반갑습니다."};
    static std::uniform_int_distribution<> dist(0, messages.size() - 1);

    auto& message = messages.at(dist(gen));
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
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device     device;
    static std::mt19937           gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
    this->send(fb::protocol::game::request::direction{direction});
    co_return;
}

async::task<void> game_bot::pattern_move()
{
    static std::vector<DIRECTION> directions{DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};
    static std::random_device     device;
    static std::mt19937           gen(device());
    static std::uniform_int_distribution<> dist(0, directions.size() - 1);

    auto direction = directions.at(dist(gen));
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
    this->send(
        fb::protocol::game::request::board::board(BOARD_ACTION::WRITE, section, 0, 0, "게시글 타이틀", "게시글 내용"));
    this->send(fb::protocol::game::request::board::board(BOARD_ACTION::DELETE, section, 0));
    co_return;
}