-- spell: 중독
-- 중독 캐스팅
local spell = require('lib.spell')

function ON_CAST_3059(me, you, sp)
    local mp = 30
    local sound = 68
    local effect = 14
    local buff_time = 30
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = RESIST.POISON}) then
        you:buff(sp, buff_time, me)
    end
end

-- 중독 버프 효과
-- function ON_BUFF_3059(me, sp)
--
-- end

-- 중독 버프 해제 효과
-- function ON_UNBUFF_3059(me, sp)
--
-- end

-- 중독 지속 효과
function ON_CONCAST_3059(me, caster, buff)
    local damage = math.min((me:maxhp() * 5) // 100, 14994)
    if damage > me:hp() - 100 then
        damage = math.max(0, me:hp() - 100)
    end
    caster:damage_to(me, damage, { critical = false, rate = caster:skill_damage_rate() / 1000.0, physical = false })
end
