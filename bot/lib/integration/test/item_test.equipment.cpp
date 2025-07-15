#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> item_test::test_equipment(uint32_t index)
{
    fb::logger::debug("Starting scenario 1-2: Equipment test with conditions");

    static auto equipment_types = std::unordered_set<ITEM_TYPE>{ITEM_TYPE::WEAPON,
                                                                ITEM_TYPE::ARMOR,
                                                                ITEM_TYPE::HELMET,
                                                                ITEM_TYPE::RING,
                                                                ITEM_TYPE::SHIELD,
                                                                ITEM_TYPE::AUXILIARY};

    static auto equipment_parts = std::unordered_map<ITEM_TYPE, EQUIPMENT_PARTS>{
        {ITEM_TYPE::WEAPON,    EQUIPMENT_PARTS::WEAPON   },
        {ITEM_TYPE::SHIELD,    EQUIPMENT_PARTS::SHIELD   },
        {ITEM_TYPE::ARMOR,     EQUIPMENT_PARTS::ARMOR    },
        {ITEM_TYPE::HELMET,    EQUIPMENT_PARTS::HELMET   },
        {ITEM_TYPE::RING,      EQUIPMENT_PARTS::LEFT_HAND},
        {ITEM_TYPE::AUXILIARY, EQUIPMENT_PARTS::LEFT_AUX }
    };

    auto  seq    = 0;
    auto  bots   = this->get_test_bots();
    auto& bot    = bots[index];
    auto  passed = true;
    for (auto& [_, item] : this->controller.container.model.item)
    {
        if (!equipment_types.contains(item.type))
            continue;

        if (seq++ % this->bot_count != index)
            continue;

        if (item.name == "황금호박선류")
        {
            fb::logger::debug("breakpoint");
        }

        bot->chat(std::format("[{}] Try to equip: {} (no condition)", seq, item.name));
        co_await bot->create_item(item.name, 1, DEFAULT_TIMEOUT);
        auto equipped = false;
        try
        {
            if (item.condition.empty())
            {
                if (co_await bot->equip(0, item.name, DEFAULT_TIMEOUT) == false)
                {
                    bot->chat(std::format("[{}] Equip failed: {} (no condition)", seq, item.name));
                    throw std::runtime_error(std::format("Scenario 1-2: Failed to equip {}", item.name));
                }

                equipped = true;
                bot->chat(std::format("[{}] Equip success: {} (no condition)", seq, item.name));
            }
            else
            {
                bot->chat(std::format("[{}] Reverse condition for: {}", seq, item.name));
                co_await bot->reverse_condition(item.condition, DEFAULT_TIMEOUT);
                bot->chat(std::format("[{}] Try to equip: {} (condition reversed)", seq, item.name));
                if (co_await bot->equip(0, item.name, DEFAULT_TIMEOUT))
                {
                    equipped = true;
                    bot->chat(std::format("[{}] Unexpectedly equipped: {} (condition reversed)", seq, item.name));
                    throw std::runtime_error(
                        std::format("Scenario 1-2: Unexpectedly succeeded to equip {} with conditions", item.name));
                }
                else
                {
                    bot->chat(std::format("[{}] Equip failed as expected: {} (condition reversed)", seq, item.name));
                }

                bot->chat(std::format("[{}] Apply condition for: {}", seq, item.name));
                co_await bot->apply_condition(item.condition, DEFAULT_TIMEOUT);
                bot->chat(std::format("[{}] Try to equip: {} (condition applied)", seq, item.name));
                if (co_await bot->equip(0, item.name, DEFAULT_TIMEOUT) == false)
                {
                    bot->chat(std::format("[{}] Equip failed: {} (condition applied)", seq, item.name));
                    throw std::runtime_error(std::format("Scenario 1-2: Failed to equip {}", item.name));
                }
                equipped = true;
                bot->chat(std::format("[{}] Equip success: {} (condition applied)", seq, item.name));

                if (equipment_parts.contains(item.type) == false)
                {
                    bot->chat(std::format("[{}] Equip part not found: {}", seq, item.name));
                    throw std::runtime_error(
                        std::format("Scenario 1-2: Unexpectedly succeeded to equip {} with conditions", item.name));
                }
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("Scenario 1-2: {}", e.what());
            passed = false;
        }

        bot->chat(std::format("[{}] Unequip: {}", seq, item.name));
        if (equipped)
            co_await bot->unequip(equipment_parts[item.type], DEFAULT_TIMEOUT);

        bot->chat(std::format("[{}] Clear inventory after test: {}", seq, item.name));
        co_await bot->clear_inventory(DEFAULT_TIMEOUT);
    }
    co_return passed;
}

async::task<bool> item_test::test_equipment_overflow()
{
    fb::logger::debug("Starting scenario 3-1: Equip items with overflow");

    constexpr auto CONTAINER_CAPACITY = 52;

    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        co_await bot->create_item("목도", 1, DEFAULT_TIMEOUT);
        bot->chat(std::format("Scenario 3-1: Created {} 목도", i + 1));
    }
    bot->chat(std::format("Scenario 3-1: Full inventory"));

    if (co_await bot->equip(0, "목도", DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Scenario 3-1: Failed to equip 목도");
        co_return false;
    }
    bot->chat(std::format("Scenario 3-1: Equipped 1 목도 and created 1 more"));

    co_await bot->create_item("목도", 1, DEFAULT_TIMEOUT);
    bot->chat(std::format("Scenario 3-1: Full inventory again"));

    auto&& resp = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::item_inactive(EQUIPMENT_PARTS::WEAPON),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE;
        },
        DEFAULT_TIMEOUT);

    if (resp.text.find(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW)) == std::string::npos)
    {
        fb::logger::fatal("Scenario 3-1: Failed to unequip 목도");
        co_return false;
    }

    co_return true;
}