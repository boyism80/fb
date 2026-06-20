local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local NEAR_TARGET_SPELLS = {
    {
        name = "삼매진화",
        type = "TARGET",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            return current_hp, current_mp - current_mp
        end,
    },
    {
        name = "지진",
        type = "TARGET",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            return current_hp, current_mp - 580
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "NEAR TARGET DAMAGE SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(NEAR_TARGET_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(NEAR_TARGET_SPELLS) do
        local target = caster:spawn_monster_relative("다람쥐", 0, 2)
        caster:spawn_monsters_relative("다람쥐", {
            {0, 1}, {-1, 2}, {1, 2}, {0, 3},
        })

        caster:set_current_hp_mp(1000, 1000)
        local expected_hp, expected_mp = spell.calculator(caster)

        skill.request_update_internal(
            caster, resp, protocol,
            spell.type, spell_slot, "", target.oid, target.position,
            expected_hp, expected_mp)

        spell_slot = spell_slot + 1
    end

    log("debug", "NEAR TARGET DAMAGE SPELL TEST COMPLETED")
    return true
end

return M
