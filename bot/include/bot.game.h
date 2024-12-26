#ifndef __BOT_GAME_H__
#define __BOT_GAME_H__

#include <bot.h>
#include <fb/protocol/game.h>

namespace fb::bot {

class game_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;
    inline static uint32_t          _count = 0;

private:
    typedef struct _pattern_params_tag
    {
        std::function<async::task<void>()>  fn;
        std::chrono::steady_clock::duration min, max;
    } pattern_params;

private:
    bool                        _inited   = false;
    uint32_t                    _sequence = 0;
    point<uint16_t>             _position;
    fb::stream                  _transfer_buffer;
    std::vector<pattern_params> _pattern_params;
    datetime                    _next_action_time;

public:
    game_bot(bot_container& owner, uint32_t id);
    game_bot(bot_container& owner, uint32_t id, const fb::stream& params);
    ~game_bot();

private:
    template <typename Class>
    void pattern(async::task<void> (Class::*fn)(),
                 const std::chrono::steady_clock::duration& min,
                 const std::chrono::steady_clock::duration& max)
    {
        this->_pattern_params.push_back(pattern_params{std::bind(fn, static_cast<Class*>(this)), min, max});
    }

protected:
    async::task<void> on_connected() override final;
    async::task<void> on_disconnected() override final;

public:
    async::task<void> on_timer(const fb::model::datetime& now) override final;

public:
    async::task<void> handle_time(const fb::protocol::game::response::time& response);
    async::task<void> handle_state(const fb::protocol::game::response::character::state& response);
    async::task<void> handle_option(const fb::protocol::game::response::character::option& response);
    async::task<void> handle_message(const fb::protocol::game::response::message& response);
    async::task<void> handle_sequence(const fb::protocol::game::response::character::id& response);
    async::task<void> handle_spell_update(const fb::protocol::game::response::spell::update& response);
    async::task<void> handle_chat(const fb::protocol::game::response::chat& response);
    async::task<void> handle_action(const fb::protocol::game::response::life::action& response);
    async::task<void> handle_direction(const fb::protocol::game::response::object::direction& response);
    async::task<void> handle_position(const fb::protocol::game::response::character::position& response);
    async::task<void> handle_move(const fb::protocol::game::response::object::move& response);
    async::task<void> handle_map(const fb::protocol::game::response::map::config& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

public:
    async::task<void> pattern_chat();
    async::task<void> pattern_attack();
    async::task<void> pattern_direction();
    async::task<void> pattern_move();
    async::task<void> pattern_pickup();
    async::task<void> pattern_emotion();
    async::task<void> pattern_board_sections();
};

} // namespace fb::bot

#endif