#ifndef __ITEM_TEST_H__
#define __ITEM_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <stdexcept>
#include <fb/game/protocol.h>

namespace fb::bot::integration {

class item_test : public bot_integration_test
{
private:
    using super = bot_integration_test;

    struct equipment_test_data
    {
        std::string item_name;
        std::string success_message;
        std::string failure_message;
        uint8_t     required_level;
        uint8_t     required_str;
        uint8_t     required_dex;
        uint8_t     required_int;
    };

    static inline const auto type_equipment_map = std::unordered_map<fb::model::enum_value::ITEM_TYPE, EQUIPMENT_PARTS>{
        {fb::model::enum_value::ITEM_TYPE::WEAPON, EQUIPMENT_PARTS::WEAPON},
        {fb::model::enum_value::ITEM_TYPE::ARMOR,  EQUIPMENT_PARTS::ARMOR },
        {fb::model::enum_value::ITEM_TYPE::SHIELD, EQUIPMENT_PARTS::SHIELD},
        {fb::model::enum_value::ITEM_TYPE::HELMET, EQUIPMENT_PARTS::HELMET}
    };

protected:
    generator<scenario_t> on_generate_scenario() override final;
    async::task<void>     on_initialize(game_bot_controller& controller) override final;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override final;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override final;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override final;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override final;

public:
    item_test(game_bot_controller& controller);
    ~item_test() = default;
    std::string name() const override final;

private:
    async::task<bool> execute_test_functions(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots);
    async::task<bool> test_equipment(uint32_t index);
    async::task<bool> test_equipment_overflow();
    async::task<bool> test_item_combine(uint32_t index);
    async::task<bool> test_item_combine_failure();
};

} // namespace fb::bot::integration

#endif // __ITEM_TEST_H__