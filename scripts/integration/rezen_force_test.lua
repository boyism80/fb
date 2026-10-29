local script_step = require("integration.lib.script_step")

local MAP_NAME     = "부여미궁매표소"
local MAP_POS      = { 15, 5 }
local MOB_ID       = 1216
local SPAWN_WAIT   = 2500
local RESPAWN_WAIT = 3000

local function fail(message)
    log("fatal", "Rezen force test: " .. message)
    return false
end

local function count_mobs(bot, step)
    local pass, text = script_step.run_script(bot, "test/integration", "count_mobs", step, tostring(MOB_ID))
    if pass ~= true then
        return nil
    end
    return tonumber(string.match(text or "", "count=(%d+)"))
end

test_suite {
    name      = "Rezen Force Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:map_move(MAP_NAME, MAP_POS[1], MAP_POS[2])

            bot:chat("/리스폰")
            ctx:sleep(SPAWN_WAIT)
            bot:chat("/리스폰")
            ctx:sleep(SPAWN_WAIT)

            local before = count_mobs(bot, "rezen_force_before")
            if before == nil or before < 1 then
                return fail("no mob " .. MOB_ID .. " after the forced respawn, count=" .. tostring(before))
            end

            if script_step.run_script(bot, "test/integration", "kill_mob", "rezen_force_kill", tostring(MOB_ID)) ~= true then
                return fail("could not kill mob " .. MOB_ID)
            end
            ctx:sleep(RESPAWN_WAIT)

            local after = count_mobs(bot, "rezen_force_after")
            if after ~= before - 1 then
                return fail(string.format("the killed mob came back before its rezen interval: before=%s after=%s",
                    tostring(before), tostring(after)))
            end
            return true
        end,
    },
}
