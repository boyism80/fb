local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local DAMAGE_SPELLS = {
    { name = "뢰진주",       type = "TARGET", expected_damage = 320,  expected_mp_cost = 120},
    { name = "화염주",       type = "TARGET", expected_damage = 320,  expected_mp_cost = 120},
    { name = "백열주",       type = "TARGET", expected_damage = 320,  expected_mp_cost = 120},
    { name = "자무주",       type = "TARGET", expected_damage = 320,  expected_mp_cost = 120},
    { name = "뢰격주",       type = "TARGET", expected_damage = 530,  expected_mp_cost = 180},
    { name = "화영열주",     type = "TARGET", expected_damage = 530,  expected_mp_cost = 180},
    { name = "백령주",       type = "TARGET", expected_damage = 530,  expected_mp_cost = 180},
    { name = "자영무주",     type = "TARGET", expected_damage = 530,  expected_mp_cost = 180},
    { name = "뢰격참주",     type = "TARGET", expected_damage = 740,  expected_mp_cost = 250},
    { name = "화열참주",     type = "TARGET", expected_damage = 740,  expected_mp_cost = 250},
    { name = "백열참주",     type = "TARGET", expected_damage = 740,  expected_mp_cost = 250},
    { name = "자천무주",     type = "TARGET", expected_damage = 740,  expected_mp_cost = 250},
    { name = "진뢰격참주",   type = "TARGET", expected_damage = 1950, expected_mp_cost = 330},
    { name = "진화열참주",   type = "TARGET", expected_damage = 1950, expected_mp_cost = 330},
    { name = "진백열참주",   type = "TARGET", expected_damage = 1950, expected_mp_cost = 330},
    { name = "진자천무주",   type = "TARGET", expected_damage = 1950, expected_mp_cost = 330},
    { name = "극진뢰격참주", type = "TARGET", expected_damage = 3580, expected_mp_cost = 360},
    { name = "극진화열참주", type = "TARGET", expected_damage = 3580, expected_mp_cost = 360},
    { name = "극진백열참주", type = "TARGET", expected_damage = 3580, expected_mp_cost = 360},
    { name = "극진자천무주", type = "TARGET", expected_damage = 3580, expected_mp_cost = 360},
    { name = "흡성대법",     type = "TARGET", expected_damage = 3000, expected_mp_cost = 60 },
    { name = "백열장",       type = "TARGET", expected_damage = 280,  expected_mp_cost = 80 },
    { name = "헬파이어",     type = "TARGET", expected_damage = 0,    expected_mp_cost = 0  },
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "DAMAGE SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(DAMAGE_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(DAMAGE_SPELLS) do
        local mob = caster:spawn_monster_relative("다람쥐", 0, 1)
        caster:set_current_hp_mp(50, 100000)

        local before_caster_hp = caster:hp()
        local before_caster_mp = caster:mp()
        local expected_hp = before_caster_hp
        local expected_mp = before_caster_mp - spell.expected_mp_cost

        if spell.name == "헬파이어" then
            expected_mp = 0
        end
        if spell.name == "흡성대법" then
            expected_hp = before_caster_hp + 15
        end

        log("debug", "Testing " .. spell.name)
        skill.request_update_internal(
            caster, resp, protocol,
            spell.type, spell_slot, "", mob.oid, mob.position,
            expected_hp, expected_mp)

        spell_slot = spell_slot + 1
    end

    log("debug", "DAMAGE SPELL TEST COMPLETED")
    return true
end

return M
