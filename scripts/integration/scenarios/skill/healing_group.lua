local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")
local group    = require("integration.lib.group")
local lib      = require("integration.lib")

local M = {}

local GROUP_HEALING_SPELLS = {
    {
        name = "백호의희원'첨",
        calculator = function(bot)
            local current_mp = bot:mp()
            return current_mp * 2, math.floor(current_mp / 2)
        end,
    },
    {
        name = "신령의기원'첨",
        calculator = function(_)
            return 10000, 1000
        end,
    },
}

local function verify_group_healing_effects(ctx, before_hp_values, expected_hp_gain)
    for i = 1, ctx:bot_count() - 1 do
        local bot = ctx:bot(i)
        local max_hp = bot:base_hp()
        local recoverable_amount = max_hp - before_hp_values[i + 1]
        local actual_hp_gain = math.min(expected_hp_gain, recoverable_amount)
        local expected_hp = before_hp_values[i + 1] + actual_hp_gain
        local actual_hp = bot:hp()

        if actual_hp ~= expected_hp then
            log("debug", string.format(
                "group heal verify failed: bot[%d] name=%s before=%d expected=%d actual=%d gain=%d",
                i, bot:name(), before_hp_values[i + 1], expected_hp, actual_hp, expected_hp_gain))
            return false
        end
    end
    return true
end

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "Starting group healing spell test")

    local names = skill.spell_names(GROUP_HEALING_SPELLS)
    caster:learn_spells(names)

    group.form(ctx)

    local spell_slot = 0
    local success = true

    for _, spell_info in ipairs(GROUP_HEALING_SPELLS) do
        caster:set_current_hp_mp(10000, 10000)
        local expected_hp_gain, expected_mp_cost = spell_info.calculator(caster)

        for i = 1, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:base_hp(100000)
            bot:set_current_hp_mp(50, bot:mp())
        end

        local before_hp_values = {}
        for i = 0, ctx:bot_count() - 1 do
            table.insert(before_hp_values, ctx:bot(i):hp())
        end

        local before_caster_mp = caster:mp()
        local expected_caster_mp = before_caster_mp - expected_mp_cost

        log("debug", string.format(
            "group heal cast: spell=%s slot=%d expected_hp_gain=%d expected_mp_cost=%d caster_mp=%d",
            spell_info.name, spell_slot, expected_hp_gain, expected_mp_cost, before_caster_mp))

        for i = 1, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            log("debug", string.format(
                "group heal member[%d]: name=%s oid=%d hp=%d base_hp=%d",
                i, bot:name(), bot:oid(), bot:hp(), bot:base_hp()))
        end

        skill.request_update_internal(
            caster, resp, protocol,
            "NORMAL", spell_slot, "", 0, {0, 0},
            nil, expected_caster_mp)

        log("debug", string.format(
            "group heal cast done: spell=%s caster_mp=%d (expected=%d)",
            spell_info.name, caster:mp(), expected_caster_mp))

        ctx:sleep(500)

        if caster:mp() ~= expected_caster_mp then
            log("debug", string.format(
                "group heal mp mismatch: spell=%s before=%d after=%d expected_cost=%d",
                spell_info.name, before_caster_mp, caster:mp(), expected_mp_cost))
            success = false
        elseif verify_group_healing_effects(ctx, before_hp_values, expected_hp_gain) == false then
            success = false
        else
            log("debug", "group heal passed: " .. spell_info.name)
        end

        spell_slot = spell_slot + 1
    end

    group.cleanup(ctx)
    log("debug", "All group healing spell tests completed successfully!")
    return success
end

return M
