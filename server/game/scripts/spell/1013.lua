-- spell: 진백호령
-- 진백호령 캐스팅
local spell = require('lib.spell')

-- 진백호령 버프 효과

-- 진백호령 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 30
        local sound = 89
        local effect = 57
        local buff_time = 60

        if not spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            return
        end
        me:buff(sp, buff_time)
    end,

    ON_BUFF = function(me, sp)
        me:skill_damage_rate(me:skill_damage_rate() + 500)
    end,

    ON_UNBUFF = function(me, sp)
        me:skill_damage_rate(me:skill_damage_rate() - 500)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
