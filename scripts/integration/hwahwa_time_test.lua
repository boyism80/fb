local script_step = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local HWAHWA_NPC = "화화"
local HWAHWA_MAP = "폭염도"
local HWAHWA_MAP_ID = 1698
local HWAHWA_X = 12
local HWAHWA_Y = 88
local MSG_HWAHWA_GONE = "귀신의 모습으로 돌아가야해"
local CLOCK_WAIT_MS = 1500

local function fail(message)
    log("fatal", "Hwahwa time test: " .. message)
    return false
end

test_suite({
    name = "Hwahwa Time Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        quest_dialog.remove_npc(bot)
        bot:chat("/현재시간초기화")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)

            a:map_move(HWAHWA_MAP, HWAHWA_X, HWAHWA_Y)
            if a:map() ~= HWAHWA_MAP_ID then
                return fail("could not move to the hwahwa map")
            end
            a:chat("/현재시간 2026-10-22 13:15:00")
            ctx:sleep(CLOCK_WAIT_MS)
            if script_step.run_script(a, "test/integration", "verify_clock_hour", "hwahwa_clock", "13") ~= true then
                return fail("could not set the clock to 13:15")
            end

            local packet = quest_dialog.click(a, HWAHWA_NPC, function(p)
                return p.message ~= nil or p.type == "list" or p.type == "pursuit"
            end)
            quest_dialog.close(a)
            if packet == nil then
                return fail("hwahwa did not answer")
            end
            if packet.message ~= nil and packet.message:find(MSG_HWAHWA_GONE, 1, true) ~= nil then
                return fail("hwahwa left at 13:15, inside her time window")
            end
            return true
        end,
    },
})
