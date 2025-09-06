#ifndef __ITEM_TEST_GIVE_H__
#define __ITEM_TEST_GIVE_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class item_test_give : public bot_integration_test
{
private:
    using super = bot_integration_test;

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    item_test_give(game_bot_controller& controller);
    ~item_test_give() = default;
    std::string name() const override final;

private:
    async::task<bool> test_give_non_tradeable_item();
    async::task<bool> test_give_tradeable_item();
    async::task<bool> test_give_item_inventory_full();
    async::task<bool> test_give_max_money();
    async::task<bool> test_give_to_mob_and_kill();
};

} // namespace fb::bot::integration

#endif // __ITEM_TEST_GIVE_H__