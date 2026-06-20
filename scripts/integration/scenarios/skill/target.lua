local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local TARGET_SPELLS = {
    { name = "공력주입", expected_mp_cost = 0  },
    { name = "대지의빛", expected_mp_cost = 30 },
    { name = "부활",     expected_mp_cost = 300},
    { name = "시력회복", expected_mp_cost = 40 },
    { name = "퇴마주",   expected_mp_cost = 30 },
    { name = "파혼술",   expected_mp_cost = 30 },
    { name = "해독",     expected_mp_cost = 30 },
    { name = "활력",     expected_mp_cost = 30 },
}

function M.run(ctx, bot_index, target_index)
    target_index = target_index or 6

    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)

    log("debug", "TARGET SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(TARGET_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(TARGET_SPELLS) do
        log("debug", "Testing " .. spell.name)

        local target_oid = target:oid()
        local target_pos = target:position()

        skill.cast_until_ready(
            caster, resp, protocol,
            "TARGET", spell_slot,
            target_oid, target_pos,
            spell.name,
            { target = target, mp_cost = spell.expected_mp_cost })

        spell_slot = spell_slot + 1
    end

    log("debug", "TARGET SPELL TEST COMPLETED")
    return true
end

return M
