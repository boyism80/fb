#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> item_test::test_equipment_success(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::info("Starting scenario 1-1: Equip items without conditions");

    auto equipments = std::vector<equipment_item_data>{
        {"쇠도끼",           "w:무기  :쇠도끼",           EQUIPMENT_PARTS::WEAPON, nullptr, nullptr},
        {"초심자의방패",     "s:방패  :초심자의방패",     EQUIPMENT_PARTS::SHIELD, nullptr, nullptr},
        {"초심자의머리띠",   "h:머리  :초심자의머리띠",   EQUIPMENT_PARTS::HELMET, nullptr, nullptr},
        {"초심자의남자갑주", "a:갑옷  :초심자의남자갑주", EQUIPMENT_PARTS::ARMOR,  nullptr, nullptr}
    };

    // Set basic stats for testing
    co_await bot->change_level(10, DEFAULT_TIMEOUT);
    co_await bot->change_stats(10, 10, 10, DEFAULT_TIMEOUT);
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    auto slot = 0;
    for (const auto& item : equipments)
    {
        co_await bot->create_item(item.item_name, 1, DEFAULT_TIMEOUT);
        auto success = co_await bot->equip(slot++, item.success_message, DEFAULT_TIMEOUT);
        if (!success)
        {
            fb::logger::fatal("Scenario 1-1: Failed to equip {}", item.item_name);
            co_return false;
        }

        if (co_await bot->unequip(item.equipment_part, DEFAULT_TIMEOUT) == false)
        {
            fb::logger::fatal("Scenario 1-1: Failed to unequip {}", item.item_name);
            co_return false;
        }
        fb::logger::info("Scenario 1-1: Successfully equipped {}", item.item_name);
    }

    co_return true;
}

async::task<bool> item_test::test_equipment_failure(uint32_t index)
{
    fb::logger::info("Starting scenario 2-1: Equip items with conditions - expect failure");

    // Set low stats to ensure failure
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    co_await bot->change_level(1, DEFAULT_TIMEOUT);
    co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    auto equipments = std::vector<std::vector<equipment_item_data>>{
        {
         {"검성기검",
             "w:무기  :검성기검",
             EQUIPMENT_PARTS::WEAPON,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_class("검성", timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
             }},
         {"검황의영혼",
             "a:갑옷  :검황의영혼",
             EQUIPMENT_PARTS::ARMOR,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_class("검황", timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::MAN, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::WOMAN, timeout);
             }},
         {"팔세지도",
             "s:방패  :팔세지도",
             EQUIPMENT_PARTS::SHIELD,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(90, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
             }},
         },
        {
         {"태성태도",
             "w:무기  :태성태도",
             EQUIPMENT_PARTS::WEAPON,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_class("태성", timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
             }},
         {"귀검의영혼",
             "a:갑옷  :귀검의영혼",
             EQUIPMENT_PARTS::ARMOR,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_class("귀검", timeout);
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::MAN, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::WOMAN, timeout);
             }},
         {"여신의방패",
             "s:방패  :여신의방패",
             EQUIPMENT_PARTS::SHIELD,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(77, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
             }},
         },
        {
         {"진선역봉",
             "w:무기  :진선역봉",
             EQUIPMENT_PARTS::WEAPON,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_class("진선", timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
             }},
         {"진인의심장",
             "a:갑옷  :진인의심장",
             EQUIPMENT_PARTS::ARMOR,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_class("진인", timeout);
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::WOMAN, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::MAN, timeout);
             }},
         {"문신방패",
             "s:방패  :문신방패",
             EQUIPMENT_PARTS::SHIELD,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(20, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
             }},
         },
        {
         {"현자금봉",
             "w:무기  :현자금봉",
             EQUIPMENT_PARTS::WEAPON,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_class("현자", timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
             }},
         {"현인의심장",
             "a:갑옷  :현인의심장",
             EQUIPMENT_PARTS::ARMOR,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_class("현인", timeout);
                 co_await bot.change_level(99, timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::WOMAN, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
                 co_await bot.change_class("평민", timeout);
                 co_await bot.change_sex(fb::model::enum_value::SEX::MAN, timeout);
             }},
         {"정화의방패",
             "s:방패  :정화의방패",
             EQUIPMENT_PARTS::SHIELD,
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(50, timeout);
             },
             [timeout = DEFAULT_TIMEOUT](auto& bot) -> async::task<void> {
                 co_await bot.change_level(1, timeout);
             }},
         }
    };

    co_await bot->change_level(1, DEFAULT_TIMEOUT);
    co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    auto slot = 0;
    for (const auto& item : equipments[index])
    {
        if (item.rollback != nullptr)
        {
            co_await item.rollback(*bot);
        }

        co_await bot->create_item(item.item_name, 1, DEFAULT_TIMEOUT);
        auto success = co_await bot->equip(slot, item.success_message, DEFAULT_TIMEOUT);
        if (success)
        {
            fb::logger::fatal("Scenario 2-1: Unexpectedly succeeded to equip {} with insufficient conditions",
                              item.item_name);
            co_return false;
        }

        if (item.condition != nullptr)
        {
            co_await item.condition(*bot);
        }

        success = co_await bot->equip(slot, item.success_message, DEFAULT_TIMEOUT);
        if (!success)
        {
            fb::logger::fatal("Scenario 2-1: Failed to equip {}", item.item_name);
            co_return false;
        }

        if (co_await bot->unequip(item.equipment_part, DEFAULT_TIMEOUT) == false)
        {
            fb::logger::fatal("Scenario 2-1: Failed to unequip {}", item.item_name);
            co_return false;
        }

        if (item.rollback != nullptr)
        {
            co_await item.rollback(*bot);
        }
        slot++;
    }

    co_return true;
}

async::task<bool> item_test::test_equipment_overflow()
{
    fb::logger::info("Starting scenario 3-1: Equip items with overflow");

    constexpr auto CONTAINER_CAPACITY = 52;

    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        co_await bot->create_item("목도", 1, DEFAULT_TIMEOUT);
        bot->chat(std::format("Scenario 3-1: Created {} 목도", i + 1));
    }
    bot->chat(std::format("Scenario 3-1: Full inventory"));

    if (co_await bot->equip(0, "w:무기  :목도", DEFAULT_TIMEOUT) == false)
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