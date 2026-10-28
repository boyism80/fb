#ifndef __BOT_GAME_LOAD_CONTROLLER_H__
#define __BOT_GAME_LOAD_CONTROLLER_H__

#include <fb/bot/game_bot.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/load/brain.h>
#include <fb/bot/load/knowledge.h>
#include <fb/game/protocol.h>

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace fb::bot::load {

class game_bot_controller : public fb::bot::game_bot_controller
{
private:
    using super = fb::bot::game_bot_controller;

public:
    using bot_type = game_bot; ///< Type alias for the managed bot type

private:
    std::unique_ptr<knowledge>                              _knowledge;
    std::vector<persona>                                    _personas;
    Json::Value                                             _spread;
    tutorial                                                _tutorial;
    job                                                     _job;
    supplies                                                _supplies;
    spellbook                                               _spellbook;
    questbook                                               _questbook;
    hazards                                                 _hazards;
    market                                                  _market;
    roster                                                  _roster;
    crowd                                                   _crowd;
    std::vector<std::string>                                _setup;
    census                                                  _census;
    experience                                              _experience;
    messages                                                _messages;
    trace                                                   _trace;
    metrics                                                 _metrics;
    std::mutex                                              _brains_mutex;
    std::unordered_map<std::string, std::shared_ptr<brain>> _brains;

public:
    game_bot_controller(bot_container& container);

    void initialize();

public:
    virtual async::task<void> on_bot_connected(game_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;

private:
    async::task<void> on_timer(const fb::model::datetime& now, std::thread::id id);
    async::task<void> on_metrics();
    async::task<void> on_sync();
};

} // namespace fb::bot::load

#endif
