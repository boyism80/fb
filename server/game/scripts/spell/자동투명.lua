-- 자동투명 캐스팅
local spell = require('lib.spell')

function ON_CAST_13(me, sp)
    local buff_time = 180
    if me:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
        return false
    end

    me:buff(sp, buff_time)
end

-- 자동투명 버프 효과
function ON_BUFF_13(me, sp)
    me:state(STATE.HALF_CLOACK)
end

-- 자동투명 버프 해제 효과
function ON_UNBUFF_13(me, sp)
    me:state(STATE.NORMAL)

end