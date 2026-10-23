local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local MAP             = "낙랑의방"
local A_POS           = { 6, 6 }
local B_POS           = { 8, 6 }
local ROLE_USER       = 0
local ROLE_ADMIN      = 2
local MSG_NO_APPEAR   = "그곳으로는 출두할 수 없습니다."
local MSG_NO_SUMMON   = "이곳으로는 소환할 수 없습니다."
local MSG_CAST        = "외웠습니다"
local MESSAGE_WAIT_MS = 10000

local function fail(message)
    log("fatal", "Instance teleport test: " .. message)
    return false
end

local function warp(ctx, bot, x, y, slot, step)
    local command = string.format("/맵이동 %s %d %d", MAP, x, y)
    if slot ~= nil then
        command = string.format("%s %d", command, slot)
    end
    bot:chat(command)
    ctx:sleep(1000)
    local expected = slot ~= nil and "1" or "0"
    return script_step.run_script(bot, "test/integration", "verify_map_instance", step, expected) == true
end

local function set_role(admin, bot, role, step)
    return script_step.run_script(admin, "test/integration", "set_role", step, bot:name(), tostring(role)) == true
end

local function prepare_spells(bot)
    bot:clear_all_spells()
    bot:learn_spells({ "출두", "소환" })
end

local function cast(caster, target_name, spell_name, expected)
    local spells = { ["출두"] = 0, ["소환"] = 1 }
    caster:take_messages()
    local packet = caster:request(resp.message,
        protocol.spell_cast("INPUT", spells[spell_name], target_name, 0, { 0, 0 }),
        function(p)
            return p.text ~= nil and p.text:find(expected, 1, true) ~= nil
        end,
        MESSAGE_WAIT_MS)
    return packet ~= nil and packet ~= false
end

test_suite {
    name      = "Instance Teleport Test",
    bot_count = 3,

    on_scenario_finished = function(ctx)
        local c = ctx:bot(2)
        set_role(c, ctx:bot(0), ROLE_ADMIN, "restore_a")
        set_role(c, ctx:bot(1), ROLE_ADMIN, "restore_b")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local c = ctx:bot(2)
            local slot = ctx:suite_slot()

            prepare_spells(a)
            prepare_spells(b)
            if warp(ctx, a, A_POS[1], A_POS[2], slot, "a_instance") == false then
                return fail("could not move A into the instance")
            end
            if warp(ctx, b, B_POS[1], B_POS[2], nil, "b_source") == false then
                return fail("could not move B to the source map")
            end
            if set_role(c, a, ROLE_USER, "demote_a") == false or set_role(c, b, ROLE_USER, "demote_b") == false then
                return fail("could not demote A and B")
            end

            if cast(b, a:name(), "출두", MSG_NO_APPEAR) == false then
                return fail("B appeared into A's instance from outside it")
            end
            if cast(a, b:name(), "소환", MSG_NO_SUMMON) == false then
                return fail("A summoned B into its instance from outside it")
            end

            if set_role(c, b, ROLE_ADMIN, "promote_b") == false then
                return fail("could not promote B to enter the instance")
            end
            if warp(ctx, b, B_POS[1], B_POS[2], slot, "b_instance") == false then
                return fail("could not move B into the instance")
            end
            if set_role(c, b, ROLE_USER, "demote_b_again") == false then
                return fail("could not demote B again")
            end

            if cast(a, b:name(), "소환", MSG_CAST) == false then
                return fail("A could not summon B inside the same instance")
            end
            if cast(b, a:name(), "출두", MSG_CAST) == false then
                return fail("B could not appear to A inside the same instance")
            end
            return true
        end,
    },
}
