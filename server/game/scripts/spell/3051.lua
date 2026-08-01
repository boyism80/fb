-- spell: 저주
-- 저주 캐스팅
local spell = require('lib.spell')

-- 저주 버프 효과

-- 저주 버프 해제 효과

return {
    on_cast = function(me, you, sp)
        local mp = 30
        local sound = 69
        local effect = 13
        local buff_time = 185
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = RESIST.CURSE}) then
            you:buff(sp, buff_time, me)
        end
    end,

    on_buff = function(me, sp)
        me:buff_phydef(me:buff_phydef() + 30)
    end,

    on_unbuff = function(me, sp)
        me:buff_phydef(me:buff_phydef() - 30)
    end,

    -- on_concast = function(me, sp)
    -- end
}
