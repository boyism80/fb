-- spell: 운기
-- 운기 캐스팅
local spell = require('lib.spell')

function ON_CAST_2024(me, sp)
    local mp = 30
    local sound = 2
    local effect = nil
    local buff_time = 11
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time, me)
    end
end

-- 운기 버프 효과
function ON_BUFF_2024(me, sp)

end

-- 운기 버프 해제 효과
function ON_UNBUFF_2024(me, sp)

end

-- 운기 지속 효과
function ON_CONCAST_2024(me, caster, buff)
    me:effect(133)
    me:sound(2)
    me:mp_up(me:maxmp() // 10)
end
