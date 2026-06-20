local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local ATTACK_CAST_SPELLS = {
    {
        name = "건곤대나이",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp * 2 / 3)
            return current_hp - hp_cost, current_mp - 40
        end,
    },
    {
        name = "동귀어진",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = current_hp - 10
            return current_hp - hp_cost, current_mp - 10
        end,
    },
    {
        name = "백호검무",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 60
        end,
    },
    {
        name = "백호참",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 60
        end,
    },
    {
        name = "필살검무",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            local mp_cost = math.max(current_mp, 60)
            return current_hp - hp_cost, current_mp - mp_cost
        end,
    },
    {
        name = "무형검",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 180
        end,
    },
    {
        name = "이기어검",
        type = "NORMAL",
        calculator = function(bot)
            local current_hp = bot:hp()
            local current_mp = bot:mp()
            local hp_cost = math.floor(current_hp / 2)
            return current_hp - hp_cost, current_mp - 180
        end,
    },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "ATTACK_CAST SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(ATTACK_CAST_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(ATTACK_CAST_SPELLS) do
        caster:spawn_monster_relative("다람쥐", 0, 1)
        caster:set_current_hp_mp(1000, 1000)

        local expected_hp, expected_mp = spell.calculator(caster)

        skill.request_update_internal(
            caster, resp, protocol,
            spell.type, spell_slot, "", 0, {0, 0},
            expected_hp, expected_mp)

        spell_slot = spell_slot + 1
    end

    log("debug", "ATTACK_CAST SPELL TEST COMPLETED")
    return true
end

return M
