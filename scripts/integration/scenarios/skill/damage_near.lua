local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local NEAR_DAMAGE_SPELLS = {
    { name = "뢰진주'첨",       type = "NORMAL", expected_damage = 300,  expected_mp_cost = 180},
    { name = "화염주'첨",       type = "NORMAL", expected_damage = 300,  expected_mp_cost = 180},
    { name = "백열주'첨",       type = "NORMAL", expected_damage = 300,  expected_mp_cost = 180},
    { name = "자무주'첨",       type = "NORMAL", expected_damage = 300,  expected_mp_cost = 180},
    { name = "뢰격주'첨",       type = "NORMAL", expected_damage = 510,  expected_mp_cost = 250},
    { name = "화영열주'첨",     type = "NORMAL", expected_damage = 510,  expected_mp_cost = 250},
    { name = "백령주'첨",       type = "NORMAL", expected_damage = 510,  expected_mp_cost = 250},
    { name = "자영무주'첨",     type = "NORMAL", expected_damage = 510,  expected_mp_cost = 250},
    { name = "뢰격참주'첨",     type = "NORMAL", expected_damage = 720,  expected_mp_cost = 330},
    { name = "화열참주'첨",     type = "NORMAL", expected_damage = 720,  expected_mp_cost = 330},
    { name = "백열참주'첨",     type = "NORMAL", expected_damage = 720,  expected_mp_cost = 330},
    { name = "자천무주'첨",     type = "NORMAL", expected_damage = 720,  expected_mp_cost = 330},
    { name = "진뢰격참주'첨",   type = "NORMAL", expected_damage = 1930, expected_mp_cost = 400},
    { name = "진화열참주'첨",   type = "NORMAL", expected_damage = 1930, expected_mp_cost = 400},
    { name = "진백열참주'첨",   type = "NORMAL", expected_damage = 1930, expected_mp_cost = 400},
    { name = "진자천무주'첨",   type = "NORMAL", expected_damage = 1930, expected_mp_cost = 400},
    { name = "극진뢰격참주'첨", type = "NORMAL", expected_damage = 3560, expected_mp_cost = 470},
    { name = "극진화열참주'첨", type = "NORMAL", expected_damage = 3560, expected_mp_cost = 470},
    { name = "극진백열참주'첨", type = "NORMAL", expected_damage = 3560, expected_mp_cost = 470},
    { name = "극진자천무주'첨", type = "NORMAL", expected_damage = 3560, expected_mp_cost = 470},
}

function M.run(ctx, bot_index)
    local caster = ctx:bot(bot_index)
    log("debug", "NEAR DAMAGE SPELL TEST STARTED")

    caster:move("LEFT")
    ctx:sleep(skill.DEFAULT_INTERVAL)
    caster:direction("BOTTOM")

    caster:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(NEAR_DAMAGE_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(NEAR_DAMAGE_SPELLS) do
        caster:spawn_monsters_bulk("다람쥐", 1)
        caster:set_current_hp_mp(50, 100000)

        local before_caster_hp = caster:hp()
        local before_caster_mp = caster:mp()
        local expected_hp = before_caster_hp
        local expected_mp = before_caster_mp - spell.expected_mp_cost

        log("debug", "Testing " .. spell.name)
        skill.request_update_internal(
            caster, resp, protocol,
            spell.type, spell_slot, "", 0, {0, 0},
            expected_hp, expected_mp)

        spell_slot = spell_slot + 1
    end

    ctx:sleep(skill.DEFAULT_INTERVAL)
    caster:move("RIGHT")
    caster:direction("BOTTOM")

    log("debug", "NEAR DAMAGE SPELL TEST COMPLETED")
    return true
end

return M
