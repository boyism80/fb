-- spell: 자동무장
-- 자동무장 캐스팅
local spell = require('lib.spell')

-- 자동무장 버프 효과

-- 자동무장 버프 해제 효과

return {
    on_cast = function(me, sp)
        if me:isbuff(table.unpack(spell.relative_buff_name(sp:name()))) then
            return false
        end

        me:sound(8)
        me:effect(110)
        me:buff(sp, 180, me)
    end,

    on_buff = function(me, sp)
        me:buff_phydef(me:buff_phydef() -10)
    end,

    on_unbuff = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 10)
    end,

    -- on_concast = function(me, sp)
    -- end
}
