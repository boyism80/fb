-- spell: 저주
-- 저주 캐스팅
local spell = require('lib.spell')

-- 저주 버프 효과

-- 저주 버프 해제 효과

return {
    ON_CAST = function(me, you, sp)
        local mp = 30
        local sound = 69
        local effect = 13
        local buff_time = 185
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = RESIST.CURSE}) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 30)
    end,

    ON_UNBUFF = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 30)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
