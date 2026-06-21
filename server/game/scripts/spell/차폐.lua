-- 차폐 캐스팅
local spell = require('lib.spell')

function ON_CAST_4005(me, sp)
    local mp = 100
    local sound = 8
    local effect = 2
    local buff_time = 21
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time)
    end
end

-- 차폐 버프 효과
function ON_BUFF_4005(me, sp)
    me:cover(true)
end

-- 차폐 버프 해제 효과
function ON_UNBUFF_4005(me, sp)
    me:cover(false)

end