#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> item_test::test_equipment_success(uint32_t index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    fb::logger::debug("Starting scenario 1-1: Equip items without conditions");

    auto item_names = std::vector<std::string>{"쇠도끼", "초심자의방패", "초심자의머리띠", "초심자의남자갑주"};

    // Set basic stats for testing
    co_await bot->change_level(10, DEFAULT_TIMEOUT);
    co_await bot->change_stats(10, 10, 10, DEFAULT_TIMEOUT);
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    auto slot = 0;
    for (const auto& item_name : item_names)
    {
        auto item_model = this->controller.container.model.item.name2item(item_name);
        if (!item_model)
        {
            fb::logger::fatal("Scenario 1-1: Item model not found for {}", item_name);
            co_return false;
        }

        // Get equipment part from model type
        auto equipment_part = type_equipment_map.at(item_model->type);

        co_await bot->create_item(item_name, 1, DEFAULT_TIMEOUT);
        auto success = co_await bot->equip(slot++, item_name, DEFAULT_TIMEOUT);
        if (!success)
        {
            fb::logger::fatal("Scenario 1-1: Failed to equip {}", item_name);
            co_return false;
        }

        if (co_await bot->unequip(equipment_part, DEFAULT_TIMEOUT) == false)
        {
            fb::logger::fatal("Scenario 1-1: Failed to unequip {}", item_name);
            co_return false;
        }
        fb::logger::debug("Scenario 1-1: Successfully equipped {}", item_name);
    }

    co_return true;
}

async::task<bool> item_test::test_equipment_failure(uint32_t index)
{
    fb::logger::debug("Starting scenario 2-1: Equip items with conditions - expect failure");

    auto  bots = this->get_test_bots();
    auto& bot  = bots[index];

    // Test item sets for different scenarios
    auto item_sets = std::vector<std::vector<std::string>>{
        {"검성기검", "검황의영혼", "팔세지도"  },
        {"태성태도", "귀검의영혼", "여신의방패"},
        {"진선역봉", "진인의심장", "문신방패"  },
        {"현자금봉", "현인의심장", "정화의방패"}
    };

    // Set low stats to ensure failure
    co_await bot->change_level(1, DEFAULT_TIMEOUT);
    co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    auto slot = 0;
    for (const auto& item_name : item_sets[index])
    {
        auto item_model = this->controller.container.model.item.name2item(item_name);
        if (!item_model)
        {
            fb::logger::fatal("Scenario 2-1: Item model not found for {}", item_name);
            co_return false;
        }

        auto& equipment_model = static_cast<fb::model::equipment&>(*item_model);

        // Get equipment part from model type
        auto equipment_part = type_equipment_map.at(item_model->type);

        // Try to equip with insufficient conditions (should fail)
        co_await bot->create_item(item_name, 1, DEFAULT_TIMEOUT);
        auto success = co_await bot->equip(slot, item_name, DEFAULT_TIMEOUT);
        if (success)
        {
            fb::logger::fatal("Scenario 2-1: Unexpectedly succeeded to equip {} with insufficient conditions",
                              item_name);
            co_return false;
        }

        // Apply conditions from model
        auto required_class     = CLASS::NONE;
        auto required_promotion = 0;
        for (const auto& condition : item_model->condition)
        {
            switch (condition.header)
            {
            case DSL::class_t:
            {
                auto params    = dsl::class_t(condition.params);
                required_class = params.value;
            }
            break;

            case DSL::promotion:
            {
                auto params        = dsl::promotion(condition.params);
                required_promotion = params.value;
            }
            break;

            case DSL::level:
            {
                auto params = dsl::level(condition.params);
                co_await bot->change_level(params.min.value(), DEFAULT_TIMEOUT);
            }
            break;

            case DSL::sex:
            {
                auto params = dsl::sex(condition.params);
                co_await bot->change_sex(params.value, DEFAULT_TIMEOUT);
            }
            break;
            }
        }

        static auto class_names = std::unordered_map<CLASS, std::unordered_map<uint8_t, std::string>>{
            {CLASS::WARRIOR, {{0, "전사"}, {1, "검객"}, {2, "검제"}, {3, "검황"}, {4, "검성"}}  },
            {CLASS::MAGE,    {{0, "주술사"}, {1, "술사"}, {2, "현사"}, {3, "현인"}, {4, "현자"}}},
            {CLASS::ROGUE,   {{0, "도적"}, {1, "자객"}, {2, "진검"}, {3, "귀검"}, {4, "태성"}}  },
            {CLASS::POET,    {{0, "도사"}, {1, "도인"}, {2, "명인"}, {3, "진인"}, {4, "진선"}}  }
        };

        if (required_class != CLASS::NONE)
        {
            auto class_name = std::string{};
            co_await bot->change_class(class_names[required_class][required_promotion], DEFAULT_TIMEOUT);
        }

        // Try to equip with proper conditions (should succeed)
        success = co_await bot->equip(slot, item_name, DEFAULT_TIMEOUT);
        if (!success)
        {
            fb::logger::fatal("Scenario 2-1: Failed to equip {} with proper conditions", item_name);
            co_return false;
        }

        if (co_await bot->unequip(equipment_part, DEFAULT_TIMEOUT) == false)
        {
            fb::logger::fatal("Scenario 2-1: Failed to unequip {}", item_name);
            co_return false;
        }

        // Reset to low stats for next item
        co_await bot->change_level(1, DEFAULT_TIMEOUT);
        co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);
        co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

        slot++;
    }

    co_return true;
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