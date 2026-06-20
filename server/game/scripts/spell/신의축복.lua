-- 신의축복 캐스팅
local spell = require('lib.spell')

function ON_CAST_1005(me, sp)
    local mp = 30
    local sound = 6
    local effect = 58
    local buff_time = 600
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time)
    end
end

-- 신의축복 버프 효과
function ON_BUFF_1005(me, sp)
    me:hit(me:hit()+3)
end

-- 신의축복 버프 해제 효과
function ON_UNBUFF_1005(me, sp)
    me:hit(me:hit()-3)

end