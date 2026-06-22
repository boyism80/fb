-- spell: 절망
-- 절망 캐스팅
local spell = require('lib.spell')

function ON_CAST_3053(me, you, sp)
    local mp = 80
    local sound = 71
    local effect = 14
    local buff_time = 10
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 절망 버프 효과
function ON_BUFF_3053(me, sp)
    me:add_cc(CROWD_CONTROL.SIGHT)
end

-- 절망 버프 해제 효과
function ON_UNBUFF_3053(me, sp)

    me:remove_cc(CROWD_CONTROL.SIGHT)
end

-- 절망 지속 효과
function ON_CONCAST_3053(me, caster, buff)

end

