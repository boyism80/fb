local resp     = require("integration.response")
local protocol = require("integration.protocol")
local skill    = require("integration.lib.skill")

local M = {}

local BUFF_DEBUFF_SPELLS = {
  -- Debuff spells
    { name = "혼마술",   type = "TARGET", is_self_cast = false, expected_mp_cost = 40,  target_type = "LIFE" },
    { name = "중독",     type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "절망",     type = "TARGET", is_self_cast = false, expected_mp_cost = 80,  target_type = "LIFE" },
    { name = "저주",     type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "마비",     type = "TARGET", is_self_cast = false, expected_mp_cost = 80,  target_type = "LIFE" },
    { name = "노도성황", type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
  -- Self buff spells
    { name = "투명",     type = "NORMAL", is_self_cast = true,  expected_mp_cost = 0,   target_type = "LIFE" },
    { name = "신의축복", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "백호령",   type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "진백호령", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "측면공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 90,  target_type = "LIFE" },
    { name = "후면공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 90,  target_type = "LIFE" },
    { name = "차폐",     type = "NORMAL", is_self_cast = true,  expected_mp_cost = 100, target_type = "LIFE" },
    { name = "전혈",     type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "운기",     type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "심안투영", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "파력무참", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 0,   target_type = "LIFE" },
    { name = "신령지익", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 0,   target_type = "LIFE" },
    { name = "이중공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 60,  target_type = "LIFE" },
    { name = "삼중공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 60,  target_type = "LIFE" },
    { name = "사중공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 150, target_type = "LIFE" },
    { name = "오중공격", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 200, target_type = "LIFE" },
    { name = "분신",     type = "NORMAL", is_self_cast = true,  expected_mp_cost = 0,   target_type = "LIFE" },
    { name = "누리의힘", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "금강불체", type = "NORMAL", is_self_cast = true,  expected_mp_cost = 600, target_type = "LIFE" },
  -- Target buff spells
    { name = "무장",     type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "보호",     type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
    { name = "망각",     type = "TARGET", is_self_cast = false, expected_mp_cost = 60,  target_type = "MOB"  },
    { name = "대지의힘", type = "TARGET", is_self_cast = false, expected_mp_cost = 30,  target_type = "LIFE" },
}

function M.run(ctx, bot_index, target_index)
    target_index = target_index or 5

    local caster = ctx:bot(bot_index)
    local target = ctx:bot(target_index)

    log("debug", "BUFF/DEBUFF SPELL TEST STARTED")

    caster:setup_bot_stats(100000, 100000)
    target:setup_bot_stats(100000, 100000)
    caster:learn_spells(skill.spell_names(BUFF_DEBUFF_SPELLS))

    local spell_slot = 0
    for _, spell in ipairs(BUFF_DEBUFF_SPELLS) do
        if spell.target_type ~= "LIFE" then
            spell_slot = spell_slot + 1
        else
            local actual_target = spell.is_self_cast and caster or target

            log("debug", "Testing " .. spell.name)

            local target_oid = actual_target:oid()
            local target_pos = actual_target:position()

            skill.cast_until_ready(caster, resp, protocol, spell.type, spell_slot, target_oid, target_pos, spell.name, {
                target = actual_target,
                mp_cost = spell.expected_mp_cost,
            })

            actual_target:request(
                resp.spell_buff,
                protocol.self_info(),
                function(packet)
                    return packet.name == spell.name
                end)

            actual_target:remove_buffs()
            spell_slot = spell_slot + 1
        end
    end

    log("debug", "BUFF/DEBUFF SPELL TEST COMPLETED")
    return true
end

return M
