-- 백호령 캐스팅
local spell = require('lib.spell')

function ON_CAST_1008(me, sp)
    local mp = 30
    local sound = 12
    local effect = 2
    local buff_time = 600
    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time)
    end
end

-- 백호령 버프 효과
function ON_BUFF_1008(me, sp)
    me:dam(me:dam()+3)
end

-- 백호령 버프 해제 효과
function ON_UNBUFF_1008(me, sp)
    me:dam(me:dam()-3)

end