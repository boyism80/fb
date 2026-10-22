local protocol     = require("integration.protocol")
local script_step  = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local SCROLL = "낙랑의두루마리2"

local function fail(message)
    log("fatal", "Nakrang scroll test: " .. message)
    return false
end

-- Returns the exp after preparing, or nil.
local function prepare(bot, step, level)
    bot:clear_inventory()
    local ok, text = script_step.run_script(bot, "test/integration", "nakrang2_prepare", step, level)
    if ok ~= true then
        return nil
    end
    return tonumber(tostring(text):match("exp=(%d+)"))
end

local function open_reward(bot)
    local slot = bot:item_slot(SCROLL)
    if slot == nil or slot == 0xFF then
        return nil
    end
    return bot:request_dialog_ext(protocol.item_active(slot), quest_dialog.contains("잘 하셨습니다"))
end

local function verify(bot, step, completed, exp)
    return script_step.run_script(bot, "test/integration", "verify_nakrang2", step, completed, exp) == true
end

test_suite {
    name      = "Nakrang Scroll Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        quest_dialog.close(bot)
        bot:clear_inventory()
    end,

    scenarios = {
        -- The scroll cannot be dropped or given, so another script takes it during the reward dialog.
        function(ctx)
            local bot = ctx:bot(0)
            local exp = prepare(bot, "nakrang_lost_prepare", 1)
            if exp == nil then
                return fail("could not prepare the quest")
            end
            if open_reward(bot) == nil then
                return fail("the reward dialog did not open")
            end
            if script_step.run_script(bot, "test/integration", "remove_items", "nakrang_take_scroll", SCROLL, 1) ~= true then
                return fail("the scroll was not taken")
            end
            quest_dialog.press(bot, "NEXT")
            ctx:sleep(500)
            if verify(bot, "nakrang_lost", "false", exp) == false then
                return fail("the quest was completed or exp was paid without the exchange")
            end
            return true
        end,

        -- Closing the reward notice after the exchange still completes the quest.
        function(ctx)
            local bot = ctx:bot(0)
            local exp = prepare(bot, "nakrang_quit_prepare", 10)
            if exp == nil then
                return fail("could not prepare the quest")
            end
            if open_reward(bot) == nil then
                return fail("the reward dialog did not open")
            end
            if quest_dialog.next_dialog(bot, quest_dialog.contains("<보상>")) == nil then
                return fail("the reward notice did not open")
            end
            quest_dialog.close(bot)
            ctx:sleep(500)
            if verify(bot, "nakrang_quit", "true", exp) == false then
                return fail("the quest stayed in progress after the exchange")
            end
            return true
        end,
    },
}
