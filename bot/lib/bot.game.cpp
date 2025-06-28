#include <fb/bot/bot.game.h>
#include <fb/bot/bot.game.controller.h>

using namespace fb::bot;

game_bot::game_bot(bot_controller<game_bot>& controller, uint32_t id) :
    bot<game_bot>(controller, id)
{
    this->_next_action_time = fb::model::datetime();

    this->pattern(&game_bot::pattern_chat, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_attack, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_direction, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_move, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_pickup, 250ms, 1000ms);
    this->pattern(&game_bot::pattern_emotion, 250ms, 1000ms);
    // this->pattern(&game_bot::pattern_bulletin_sections, 250ms, 1000ms);
}

game_bot::game_bot(bot_controller<game_bot>& controller, uint32_t id, const fb::stream& params) :
    game_bot(controller, id)
{
    auto clone    = fb::stream{params};
    auto reader   = fb::stream_reader<>(clone);
    auto enc_type = reader.read<uint8_t>();
    auto key_size = reader.read<uint8_t>();
    auto enc_key  = new uint8_t[key_size];
    reader.read(enc_key, key_size);
    this->crt() = fb::crypto(enc_type, enc_key);
    delete[] enc_key;

    this->_transfer_buffer = params;
}

game_bot::~game_bot()
{ }

async::task<void> game_bot::process_random_pattern(const fb::model::datetime& now)
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

async::task<void> game_bot::pattern_bulletin_sections()
{
    std::random_device              device;
    std::mt19937                    gen(device());
    std::uniform_int_distribution<> dist(0, 1);

    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::SECTIONS));

    auto section = (uint32_t)dist(gen);
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLES, section));
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::ARTICLE, section, 0));
    this->send(
        fb::protocol::game::request::bulletin(BULLETIN_ACTION::WRITE, section, 0, 0, "게시글 타이틀", "게시글 내용"));
    this->send(fb::protocol::game::request::bulletin(BULLETIN_ACTION::DELETE, section, 0));
    co_return;
}