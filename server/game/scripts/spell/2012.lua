-- spell: 투명
-- 투명 캐스팅
local spell = require('lib.spell')

function ON_CAST_2012(me, sp)
    local mp = 0
    local sound = 28
    local effect = 20
    local buff_time = 180
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:state(STATE.CLOACK)
        me:buff(sp, buff_time)
    end
end

-- 투명 버프 효과
function ON_BUFF_2012(me, sp)

end

-- 투명 버프 해제 효과
function ON_UNBUFF_2012(me, sp)
    me:state(STATE.NORMAL)
end
