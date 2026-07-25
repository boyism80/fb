-- spell: 무장
-- 무장 캐스팅
local spell = require('lib.spell')

-- 무장 버프 효과

-- 무장 버프 해제 효과

return {
    ON_CAST = function(me, you, sp)
        local mp = 30
        local sound = 8
        local effect = 110
        local buff_time = 180
        if spell.buff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() -10)
    end,

    ON_UNBUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 10)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
