-- spell: 절망
-- 절망 캐스팅
local spell = require('lib.spell')

-- 절망 버프 효과

-- 절망 버프 해제 효과

-- 절망 지속 효과

return {
    ON_CAST = function(me, you, sp)
        local mp = 80
        local sound = 71
        local effect = 14
        local buff_time = 10
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = {RESIST.DESPAIR, RESIST.BLIND}}) then
            you:buff(sp, buff_time, me)
        end
    end,

    ON_BUFF = function(me, sp)
        me:add_cc(CROWD_CONTROL.SIGHT)
    end,

    ON_UNBUFF = function(me, sp)
        me:remove_cc(CROWD_CONTROL.SIGHT)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
