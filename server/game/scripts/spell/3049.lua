-- spell: 노도성황
-- 노도성황 캐스팅
local spell = require('lib.spell')

-- 노도성황 버프 효과
-- 노도성황 버프 해제 효과
-- 노도성황 지속 효과

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 106
        local effect = 96
        local buff_time = 30
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
            you:buff(sp, buff_time, me)
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    on_concast = function(me, caster, buff)
        local damage = (me:maxhp() * 5) // 100
        caster:damage_to(me, damage, { critical = false, rate = caster:skill_damage_rate() / 1000.0, physical = false })
        me:effect(96)
    end
}
