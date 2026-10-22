local protocol     = require("integration.protocol")
local script_step  = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local SAILOR_NPC     = "부여-섬선원"
local SAILOR_BOARD   = "배를 태워주세요."
local SAILOR_CONFIRM = "네, 승선시켜 주시기 바랍니다."
local MSG_DOCKED     = "배가 선착장에 정박해 있습니다."
local MSG_BOARDING   = "그럼, 선실에 승선시켜드리도록 하겠습니다."
local MSG_DEPARTED   = "배가 이미 출항했습니다."
local TICKET         = "부여-고균도승선권"
local CLOCK_WAIT_MS  = 1500

local function fail(message)
    log("fatal", "Sailor boarding test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function set_clock(ctx, bot, value, step, hour)
    bot:chat("/현재시간 " .. value)
    ctx:sleep(CLOCK_WAIT_MS)
    return run_step(bot, "verify_clock_hour", step, tostring(hour))
end

local function contains(text)
    return function(p)
        return p.message ~= nil and p.message:find(text, 1, true) ~= nil
    end
end

test_suite {
    name      = "Sailor Boarding Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        quest_dialog.remove_npc(bot)
        bot:chat("/현재시간초기화")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)

            a:chat("/레벨바꾸기 21")
            a:create_item(TICKET, 1)
            ctx:sleep(300)
            if set_clock(ctx, a, "2026-10-22 13:50:00", "sailor_docked_clock", 13) == false then
                return fail("could not set the clock to 13:50")
            end

            local list = quest_dialog.click(a, SAILOR_NPC, quest_dialog.is_list)
            if list == nil then
                return fail("the sailor menu did not open")
            end
            local docked = quest_dialog.select_option(a, list, SAILOR_BOARD, contains(MSG_DOCKED))
            if docked == nil then
                return fail("the sailor did not report the docked boat")
            end
            local ask = a:request_dialog_ext(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), quest_dialog.is_list)
            if ask == nil then
                return fail("the sailor did not ask to board")
            end
            local boarding = quest_dialog.select_option(a, ask, SAILOR_CONFIRM, contains(MSG_BOARDING))
            if boarding == nil then
                return fail("the sailor did not start boarding")
            end

            if set_clock(ctx, a, "2026-10-22 14:25:00", "sailor_departed_clock", 14) == false then
                return fail("could not set the clock to 14:25")
            end

            local guide = quest_dialog.next_dialog(a, function(p)
                return p.message ~= nil
            end)
            local result = nil
            if guide ~= nil and guide.message:find(MSG_DEPARTED, 1, true) == nil then
                result = quest_dialog.next_dialog(a, function(p)
                    return p.message ~= nil
                end)
            else
                result = guide
            end
            quest_dialog.close(a)
            ctx:sleep(500)

            if run_step(a, "verify_items", "sailor_ticket", TICKET .. "=1") == false then
                return fail("the sailor took the ticket after the boat had left")
            end
            if result == nil or result.message:find(MSG_DEPARTED, 1, true) == nil then
                return fail("the sailor did not report the departed boat")
            end
            return true
        end,
    },
}
