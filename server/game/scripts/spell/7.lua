-- spell: 자동보호
-- 자동보호 캐스팅
local spell = require('lib.spell')

function ON_CAST_7(me, sp)
    if me:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
        return false
    end

    me:sound(8)
    me:effect(110)
    me:buff(sp, 180, me)
end

-- 자동보호 버프 효과
function ON_BUFF_7(me, sp)
    me:damage_derate(me:damage_derate() + 1000)
end

-- 자동보호 버프 해제 효과
function ON_UNBUFF_7(me, sp)
    me:damage_derate(me:damage_derate() - 1000)

end
