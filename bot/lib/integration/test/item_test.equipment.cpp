#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace fb::bot::integration::test {
// Test data based on real game/json/item.json
struct item_test_data
{
    uint32_t                       item_id;
    std::string                    name;
    std::string                    expected_message;
    std::string                    type;
    std::string                    condition_type;
    std::variant<int, std::string> condition_value;
};

// Real test data from game/json/item.json
const std::vector<item_test_data> HIGH_LEVEL_ITEMS = {
    // High level weapons
    {18,    "목검",           "w:목검",           "WEAPON", "strength", 5                 },
    {19,    "사두목도",       "w:사두목도",       "WEAPON", "strength", 10                },

    // High level armor
    {600,   "연두색남자도복", "a:연두색남자도복", "ARMOR",  "sex",      std::string("MAN")},
    {601,   "연두색짧은도포", "a:연두색짧은도포", "ARMOR",  "sex",      std::string("MAN")},

    // High level shields
    {1007,  "여신의방패",     "s:여신의방패",     "SHIELD", "level",    77                },
    {1008,  "문신방패",       "s:문신방패",       "SHIELD", "level",    20                },

    // High level helmets
    {1315,  "주술투구",       "h:주술투구",       "HELMET", "level",    75                },
    {1316,  "낭아투구",       "h:낭아투구",       "HELMET", "level",    56                },

    // High level auxiliary items
    {10183, "나무화살통",     "q:나무화살통",     "BOW",    "none",     0                 }
};

const std::vector<item_test_data> LOW_LEVEL_ITEMS = {
    // Low level weapons (no conditions)
    {15,   "초심자의목도",     "w:초심자의목도",     "WEAPON", "none", 0                   },
    {17,   "목도",             "w:목도",             "WEAPON", "none", 0                   },

    // Low level armor (gender specific)
    {10,   "초심자의남자갑주", "a:초심자의남자갑주", "ARMOR",  "sex",  std::string("MAN")  },
    {11,   "초심자의여자갑주", "a:초심자의여자갑주", "ARMOR",  "sex",  std::string("WOMAN")},

    // Low level shields (no conditions)
    {14,   "초심자의방패",     "s:초심자의방패",     "SHIELD", "none", 0                   },
    {1005, "역사수호연등",     "s:역사수호연등",     "SHIELD", "none", 0                   },

    // Low level helmets (no conditions)
    {12,   "초심자의머리띠",   "h:초심자의머리띠",   "HELMET", "none", 0                   }
};

const std::vector<item_test_data> EXTREME_LEVEL_ITEMS = {
    // Extreme level items for failure testing
    {1006, "팔세지도", "s:팔세지도", "SHIELD", "level", 90},
    {1317, "진취양모", "h:진취양모", "HELMET", "level", 90},
    {1318, "현취월모", "h:현취월모", "HELMET", "level", 90}
};
} // namespace fb::bot::integration::test

async::task<bool> item_test::test_equipment_success()
{
    // Set high stats for success testing
    for (const auto& bot : this->_bots)
    {
        if (bot == nullptr)
            continue;

        // Give high level and stats to ensure success
        bot->level(99);
        bot->strength(99);
        bot->intelligence(99);
        bot->dexterity(99);
        bot->sex(fb::character::sex::MAN); // Set male for gender-specific items
    }

    bool all_passed = true;

    for (const auto& item : HIGH_LEVEL_ITEMS)
    {
        for (const auto& bot : this->_bots)
        {
            if (bot == nullptr)
                continue;

            // Give item to bot
            bot->give(item.item_id, 1);

            // Test equip
            auto success = co_await bot->equip(0, item.expected_message, DEFAULT_TIMEOUT);
            if (!success)
            {
                this->_log->error("Failed to equip {} (id: {})", item.name, item.item_id);
                all_passed = false;
                continue;
            }

            // Test unequip
            success = co_await bot->unequip(0, DEFAULT_TIMEOUT);
            if (!success)
            {
                this->_log->error("Failed to unequip {} (id: {})", item.name, item.item_id);
                all_passed = false;
            }

            // Remove item from inventory
            bot->remove(item.item_id, 1);
        }
    }

    co_return all_passed;
}

async::task<bool> item_test::test_equipment_failure()
{
    // Set low stats for failure testing
    for (const auto& bot : this->_bots)
    {
        if (bot == nullptr)
            continue;

        // Give low level and stats to ensure failure
        bot->level(1);
        bot->strength(1);
        bot->intelligence(1);
        bot->dexterity(1);
        bot->sex(fb::character::sex::WOMAN); // Set female for male-specific items
    }

    bool all_passed = true;

    for (const auto& item : EXTREME_LEVEL_ITEMS)
    {
        for (const auto& bot : this->_bots)
        {
            if (bot == nullptr)
                continue;

            // Give item to bot
            bot->give(item.item_id, 1);

            // Test equip (should fail)
            auto success = co_await bot->equip(0, item.expected_message, DEFAULT_TIMEOUT);
            if (success)
            {
                this->_log->error("Unexpectedly succeeded to equip {} (id: {}) with low stats",
                                  item.name,
                                  item.item_id);
                all_passed = false;

                // Unequip if somehow equipped
                co_await bot->unequip(0, DEFAULT_TIMEOUT);
            }

            // Remove item from inventory
            bot->remove(item.item_id, 1);
        }
    }

    co_return all_passed;
}

async::task<bool> item_test::test_equipment_deactivation()
{
    // Set appropriate stats for basic items
    for (const auto& bot : this->_bots)
    {
        if (bot == nullptr)
            continue;

        bot->level(50);
        bot->strength(50);
        bot->intelligence(50);
        bot->dexterity(50);
        bot->sex(fb::character::sex::MAN);
    }

    bool all_passed = true;

    for (const auto& item : LOW_LEVEL_ITEMS)
    {
        for (const auto& bot : this->_bots)
        {
            if (bot == nullptr)
                continue;

            // Skip gender-specific items for opposite gender
            if (item.condition_type == "sex" && std::holds_alternative<std::string>(item.condition_value) &&
                std::get<std::string>(item.condition_value) == "WOMAN")
            {
                continue;
            }

            // Give item to bot
            bot->give(item.item_id, 1);

            // Test equip
            auto success = co_await bot->equip(0, item.expected_message, DEFAULT_TIMEOUT);
            if (!success)
            {
                this->_log->error("Failed to equip {} (id: {})", item.name, item.item_id);
                all_passed = false;
                continue;
            }

            // Test unequip
            success = co_await bot->unequip(0, DEFAULT_TIMEOUT);
            if (!success)
            {
                this->_log->error("Failed to unequip {} (id: {})", item.name, item.item_id);
                all_passed = false;
            }

            // Remove item from inventory
            bot->remove(item.item_id, 1);
        }
    }

    co_return all_passed;
}

async::task<bool> item_test::test_gender_specific_items()
{
    bool all_passed = true;

    // Test male-specific items
    for (const auto& bot : this->_bots)
    {
        if (bot == nullptr)
            continue;

        bot->sex(fb::character::sex::MAN);
        bot->level(50);

        // Test male armor
        bot->give(600, 1); // 연두색남자도복
        auto success = co_await bot->equip(0, "a:연두색남자도복", DEFAULT_TIMEOUT);
        if (!success)
        {
            this->_log->error("Failed to equip male armor as male character");
            all_passed = false;
        }
        else
        {
            co_await bot->unequip(0, DEFAULT_TIMEOUT);
        }
        bot->remove(600, 1);

        // Test female armor (should fail)
        bot->give(11, 1); // 초심자의여자갑주
        success = co_await bot->equip(0, "a:초심자의여자갑주", DEFAULT_TIMEOUT);
        if (success)
        {
            this->_log->error("Unexpectedly succeeded to equip female armor as male character");
            all_passed = false;
            co_await bot->unequip(0, DEFAULT_TIMEOUT);
        }
        bot->remove(11, 1);
    }

    co_return all_passed;
}