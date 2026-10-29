local lib = require("integration.lib")
local resp = require("integration.response")
local qd = require("integration.lib.quest_dialog")

local QUEST_TOTEM_CLOTHES = 191
local DEW_MAP = "폭염도"
local DEW_MAP_ID = 1698
local DEW_X = 18
local DEW_Y = 82
local DEW_ITEM = "여신의이슬"
local MSG_DEW_FULL = "소지품이 가득 차서 여신의 이슬을 담을 수 없다."
local check = qd.check

local function fail(fmt, ...)
    log("fatal", "goddess_dew: " .. string.format(fmt, ...))
    return false
end

test_suite({
    name = "Quest Goddess Dew Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "dew_reset", QUEST_TOTEM_CLOTHES, "none")
        bot:clear_inventory()
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)

            if localhost() and check(bot, "verify_map_active", "dew_hosted", DEW_MAP) == false then
                log("debug", "Goddess dew test skipped (폭염도 runs on another game server)")
                return true
            end

            bot:map_move(DEW_MAP, DEW_X, DEW_Y)
            local function arrived()
                local pos = bot:position()
                return pos[1] == DEW_X and pos[2] == DEW_Y
            end
            lib.wait.state(bot, resp.position, arrived)
            if bot:map() ~= DEW_MAP_ID or arrived() == false then
                return fail("could not move to %s", DEW_MAP)
            end
            if check(bot, "set_state", "dew_init", "clear;q=" .. QUEST_TOTEM_CLOTHES .. ":1;fill") == false then
                return fail("could not start the quest with a full inventory")
            end
            if check(bot, "set_quest_param", "dew_water", QUEST_TOTEM_CLOTHES, "0,0,0,50") == false then
                return fail("could not set 50 drops of dew")
            end

            bot:take_messages()
            bot:move("BOTTOM")
            local told = false
            lib.wait.state(bot, resp.message, function()
                for _, text in ipairs(bot:take_messages()) do
                    if text:find(MSG_DEW_FULL, 1, true) ~= nil then
                        told = true
                    end
                end
                return told
            end, 5000)
            if told == false then
                return fail("a full inventory at 50 drops did not tell the player")
            end
            if check(bot, "verify_items", "dew_full_items", DEW_ITEM .. "=0") == false then
                return fail("%s was made with a full inventory", DEW_ITEM)
            end

            bot:clear_inventory()
            bot:move("TOP")
            lib.wait.state(bot, resp.item_update, function()
                return bot:has_item_by_name(DEW_ITEM)
            end, 5000)
            qd.close(bot)
            if check(bot, "verify_items", "dew_made_items", DEW_ITEM .. "=1") == false then
                return fail("%s was not made at 50 drops after freeing a slot", DEW_ITEM)
            end
            if check(bot, "verify_quest", "dew_quest", QUEST_TOTEM_CLOTHES, "1") == false then
                return fail("the quest step changed while making %s", DEW_ITEM)
            end

            bot:clear_inventory()
            bot:move("BOTTOM")
            lib.wait.state(bot, resp.item_update, function()
                return bot:has_item_by_name(DEW_ITEM)
            end, 3000)
            qd.close(bot)
            if check(bot, "verify_items", "dew_once_items", DEW_ITEM .. "=0") == false then
                return fail("%s was made again after the first one left the inventory", DEW_ITEM)
            end
            return true
        end,
    },
})
