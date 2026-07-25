-- spell: 혼마술
-- 혼마술 캐스팅
local spell = require('lib.spell')

-- 혼마술 버프 효과

-- 혼마술 버프 해제 효과

return {
    ON_CAST = function(me, you, sp)
        local mp = 40
        local sound = 79
        local effect = 39
        local buff_time = 425
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 50)
    end,

    ON_UNBUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 50)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
