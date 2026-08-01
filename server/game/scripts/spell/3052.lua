-- spell: 마비
-- 마비 캐스팅
local spell = require('lib.spell')

-- 마비 버프 효과

-- 마비 버프 해제 효과

return {
    on_cast = function(me, you, sp)
        local mp = 80
        local sound = 70
        local effect = 1
        local buff_time = 20
        if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect, resist = RESIST.PARALYSIS}) then
            you:buff(sp, buff_time, me)
        end
    end,

    on_buff = function(me, sp)
        me:paralysis(true)
    end,

    on_unbuff = function(me, sp)
        me:paralysis(false)
    end,

    -- on_concast = function(me, sp)
    -- end
}
