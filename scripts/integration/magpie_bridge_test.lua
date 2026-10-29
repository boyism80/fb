local resp = require("integration.response")
local protocol = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local CHILSUNG_MAP = "칠성당"
local CHILSUNG_ID = 10954
local CHILSUNG_POS = { 10, 13 }
local BRIDGE_MAP = "오작교"
local BRIDGE_ID = 10957
local BRIDGE_POS = { 88, 15 }
local BLOCK_POS = { 70, 13 }
local HOUSE_IDS = { [10958] = true, [10959] = true }
local GOSA_LAST = "부디 기꺼운 마음으로 흠향하시옵소서."
local BLESSING = "견우직녀축복"
local CLOCK_WAIT_MS = 1500
local POLL_MS = 1000
local NO_RERUN_CHECKS = 6

local MOB_STATUE = 60008
local MOB_GYEONU = 60005
local MOB_JIKNYEO = 60006
local MOB_COUPLE = 60007

local function fail(message)
    log("fatal", "Magpie bridge test: " .. message)
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

local function wait_until(ctx, timeout_ms, check)
    local waited = 0
    while check() ~= true do
        if waited >= timeout_ms then
            return false
        end
        ctx:sleep(POLL_MS)
        waited = waited + POLL_MS
    end
    return true
end

local function wait_magpie(ctx, bot, step, spec, timeout_ms)
    return wait_until(ctx, timeout_ms, function()
        return run_step(bot, "verify_magpie", step, spec)
    end)
end

local function warp(bot, map, map_id, pos)
    local packet = bot:request(
        resp.position,
        protocol.chat(false, string.format("/맵이동 %s %d %d", map, pos[1], pos[2])),
        function(p)
            return p.abs[1] == pos[1] and p.abs[2] == pos[2]
        end
    )
    return packet ~= nil and packet ~= false and bot:map() == map_id
end

local function npc_said(text)
    return function(p)
        return p.text ~= nil and p.text:find(text, 1, true) ~= nil
    end
end

local function mob_spec(statue, gyeonu, jiknyeo, couple)
    return string.format(
        "mob%d=%d,mob%d=%d,mob%d=%d,mob%d=%d",
        MOB_STATUE,
        statue,
        MOB_GYEONU,
        gyeonu,
        MOB_JIKNYEO,
        jiknyeo,
        MOB_COUPLE,
        couple
    )
end

test_suite({
    name = "Magpie Bridge Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        run_step(bot, "magpie_walk_without_support", "magpie_support_off", "false")
        bot:chat("/현재시간초기화")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)

            if set_clock(ctx, a, "2026-08-19 11:59:40", "magpie_clock_before", 11) == false then
                return fail("could not set the clock to 칠석 11:59:40")
            end
            if run_step(a, "magpie_walk_without_support", "magpie_support_on", "true") == false then
                return fail("could not let the partners walk without support")
            end
            if
                wait_magpie(
                    ctx,
                    a,
                    "magpie_idle",
                    "check=0,warped=false,baek_done=false,remaining=none," .. mob_spec(0, 0, 0, 0),
                    10000
                ) == false
            then
                return fail("the ceremony state was not idle before 12:00")
            end
            if warp(a, CHILSUNG_MAP, CHILSUNG_ID, CHILSUNG_POS) == false then
                return fail("could not enter 칠성당")
            end

            a:chat("/현재시간 2026-08-19 12:00:00")
            local last = a:request(resp.chat, nil, npc_said(GOSA_LAST), 20000)
            if last == nil or last == false then
                return fail("백남인 did not finish the 고사 at 12:00")
            end
            if
                wait_until(ctx, 10000, function()
                    return HOUSE_IDS[a:map()] == true
                end) == false
            then
                return fail(string.format("the 고사 did not send the bot to 견우/직녀의집, map=%d", a:map()))
            end
            if
                wait_magpie(
                    ctx,
                    a,
                    "magpie_warped",
                    "warped=true,baek_done=true,baek_running=false,remaining=positive",
                    5000
                ) == false
            then
                return fail("the 고사 did not start the bridge build")
            end

            if warp(a, CHILSUNG_MAP, CHILSUNG_ID, CHILSUNG_POS) == false then
                return fail("could not return to 칠성당")
            end
            for _ = 1, NO_RERUN_CHECKS do
                if
                    run_step(
                        a,
                        "verify_magpie",
                        "magpie_no_rerun",
                        "warped=true,baek_done=true,baek_running=false,remaining=positive"
                    ) == false
                then
                    return fail("the 고사 ran again or the ceremony was reset in the same hour")
                end
                ctx:sleep(POLL_MS)
            end
            if a:map() ~= CHILSUNG_ID then
                return fail(string.format("the bot was warped again in the same hour, map=%d", a:map()))
            end

            if warp(a, BRIDGE_MAP, BRIDGE_ID, BRIDGE_POS) == false then
                return fail("could not enter 오작교")
            end
            if set_clock(ctx, a, "2026-08-19 12:06:00", "magpie_clock_built", 12) == false then
                return fail("could not set the clock to 12:06:00")
            end
            if
                wait_magpie(ctx, a, "magpie_partners", "remaining=zero,check=5,mob" .. MOB_STATUE .. "=2", 15000)
                == false
            then
                return fail("the statues and partners did not appear after the bridge build")
            end
            if warp(a, BRIDGE_MAP, BRIDGE_ID, BLOCK_POS) == false then
                return fail("could not stand on the bridge line in the partners' path")
            end
            if
                wait_until(ctx, 30000, function()
                    local pos = a:position()
                    return pos[1] ~= BLOCK_POS[1] or pos[2] ~= BLOCK_POS[2]
                end) == false
            then
                return fail("the ceremony did not clear a character standing on the bridge line")
            end
            if
                wait_magpie(
                    ctx,
                    a,
                    "magpie_reunited",
                    "reunited=true,meet_running=false," .. mob_spec(2, 0, 0, 1),
                    90000
                ) == false
            then
                return fail("견우 and 직녀 did not meet on the bridge")
            end
            if
                wait_until(ctx, 20000, function()
                    return a:has_buff(BLESSING) == true
                end) == false
            then
                return fail("견우직녀 did not bless the bot on the bridge")
            end

            if set_clock(ctx, a, "2026-08-19 12:01:00", "magpie_clock_rewind", 12) == false then
                return fail("could not rewind the clock to 12:01:00")
            end
            if
                wait_magpie(ctx, a, "magpie_rewind", "reunited=false,check=0," .. mob_spec(0, 0, 0, 0), 30000) == false
            then
                return fail("rewinding the clock did not reset the ceremony")
            end

            if set_clock(ctx, a, "2026-08-19 13:00:00", "magpie_clock_after", 13) == false then
                return fail("could not set the clock to 13:00:00")
            end
            if
                wait_magpie(
                    ctx,
                    a,
                    "magpie_closed",
                    "check=0,warped=false,remaining=none," .. mob_spec(0, 0, 0, 0),
                    15000
                ) == false
            then
                return fail("the ceremony did not close after the hour")
            end
            return true
        end,
    },
})
