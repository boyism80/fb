-- spell: 노도성황
-- 노도성황 캐스팅
local spell = require('lib.spell')

function ON_CAST_3049(me, you, sp)
    local mp = 30
    local sound = 106
    local effect = 96
    local buff_time = 30
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 노도성황 버프 효과
function ON_BUFF_3049(me, sp)

end

-- 노도성황 버프 해제 효과
function ON_UNBUFF_3049(me, sp)

end

-- 노도성황 지속 효과
function ON_CONCAST_3049(me, caster, buff)
    local damage = (me:maxhp() * 5) // 100
    me:damage(damage, caster, { critical = false, rate = caster:skill_damage_rate() / 1000.0, physical = false })
    me:effect(96)
end
