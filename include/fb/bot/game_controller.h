#ifndef __BOT_GAME_CONTROLLER_H__
#define __BOT_GAME_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/game/protocol.h>
#include <fb/bot/game_bot.h>

namespace fb::bot {

using namespace fb::model::enum_value;
namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

class game_bot_controller : public bot_controller<game_bot>
{
public:
    using bot_type = game_bot;

public:
    struct transfer_context
    {
        using promise_type = async::task_completion_source<std::shared_ptr<game_bot>>;

        std::shared_ptr<promise_type>      promise;
        std::shared_ptr<fb::timer>         timer;
        std::string                        name;
        std::atomic<bool>                  completed;
        std::weak_ptr<game_bot_controller> controller_weak;

        transfer_context(std::shared_ptr<promise_type>      promise,
                         std::string                        name,
                         std::weak_ptr<game_bot_controller> controller) :
            promise(promise),
            name(name),
            controller_weak(controller)
        { }

        void complete_success(std::shared_ptr<game_bot> bot)
        {
            if (completed.exchange(true))
                return;

            if (timer)
                timer->cancel();
            promise->set_value(bot);
        }

        void complete_timeout()
        {
            if (completed.exchange(true))
                return;

            if (auto controller = controller_weak.lock())
                controller->remove_transfer_context(name);

            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        }

        async::task<std::shared_ptr<game_bot>> task()
        {
            return promise->task();
        }
    };

private:
    std::unordered_map<std::string, std::shared_ptr<transfer_context>> _transfer_contexts;

protected:
    game_bot_controller(bot_container& container);

public:
    virtual ~game_bot_controller() = default;
    virtual void initialize()      = 0;

public:
    virtual async::task<void> on_bot_connected(game_bot& bot) override = 0;
    virtual async::task<void> on_bot_disconnected(game_bot& bot) override;
    bool                      decrypt_policy(int opcode) const override final;

private:
    async::task<void> on_time(game_bot& bot, const game_resp::time& response);
    async::task<void> on_map_config(game_bot& bot, const game_resp::map_config& response);
    async::task<void> on_state(game_bot& bot, const game_resp::update_internal& response);
    async::task<void> on_option(game_bot& bot, const game_resp::option& response);
    async::task<void> on_message(game_bot& bot, const game_resp::message& response);
    async::task<void> on_sequence(game_bot& bot, const game_resp::id& response);
    async::task<void> on_spell_update(game_bot& bot, const game_resp::spell_update& response);
    async::task<void> on_spell_remove(game_bot& bot, const game_resp::spell_remove& response);
    async::task<void> on_chat(game_bot& bot, const game_resp::chat& response);
    async::task<void> on_action(game_bot& bot, const game_resp::action& response);
    async::task<void> on_direction(game_bot& bot, const game_resp::direction& response);
    async::task<void> on_position(game_bot& bot, const game_resp::position& response);
    async::task<void> on_move(game_bot& bot, const game_resp::move& response);
    async::task<void> on_effect(game_bot& bot, const game_resp::effect& response);
    async::task<void> on_hide(game_bot& bot, const game_resp::hide& response);
    async::task<void> on_die(game_bot& bot, const game_resp::die& response);
    async::task<void> on_buff(game_bot& bot, const game_resp::spell_buff& response);
    async::task<void> on_unbuff(game_bot& bot, const game_resp::spell_unbuff& response);
    async::task<void> on_update_cc(game_bot& bot, const game_resp::update_cc& response);
    async::task<void> on_update(game_bot& bot, const game_resp::update& response);
    async::task<void> on_map(game_bot& bot, const game_resp::map_config& response);
    async::task<void> on_transfer(game_bot& bot, const fb::protocol::response::transfer& response);
    template <bool Detailed> async::task<void> on_update_external(game_bot&                                   bot,
                                                                  const game_resp::update_external<Detailed>& response)
    {
        if (bot.oid() != response.oid)
            co_return;

        if constexpr (Detailed)
        {
            bot.set_position(point<uint16_t>(response.x, response.y));
            bot.set_direction(static_cast<DIRECTION>(response.direction));
        }

        bot.set_disguised(response.disguised);
        bot.set_gender(response.gender);
        bot.set_state(response.state);
        bot.set_look(response.look);
        bot.set_color(response.color);

        bot.set_armor_dress(response.armor_dress);
        bot.set_armor_color(response.armor_color);
        bot.set_weapon_dress(response.weapon_dress);
        bot.set_weapon_color(response.weapon_color);
        bot.set_shield_dress(response.shield_dress);
        bot.set_shield_color(response.shield_color);

        bot.set_head_marker(response.head_marker);
        bot.set_name(response.name);
        co_return;
    }

private:
    async::task<void> on_item_update(game_bot& bot, const game_resp::item_update& response);
    async::task<void> on_item_remove(game_bot& bot, const game_resp::item_remove& response);
    async::task<void> on_internal_info(game_bot& bot, const game_resp::internal_info& response);
    async::task<void> on_ping(game_bot& bot, const game_resp::ping& response);

public:
    bool register_transfer_context(const fb::protocol::header& protocol, std::shared_ptr<transfer_context> context);
    void remove_transfer_context(std::string name);
    bool invoke_transfer_context(std::string name, std::shared_ptr<game_bot> bot);
};

} // namespace fb::bot

#endif