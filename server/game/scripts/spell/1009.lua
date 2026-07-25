-- spell: 이중공격
-- 이중공격 캐스팅
local spell = require('lib.spell')

-- 이중공격 버프 효과

-- 이중공격 버프 해제 효과

return {
    ON_CAST = function(me, sp)
        local mp = 60
        local sound = 9
        local effect = 70
        local buff_time = 600

        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    ON_BUFF = function(me, sp)
        me:unbuff('삼중공격')
        me:unbuff('사중공격')
        me:unbuff('오중공격')
        me:damage_rate(me:damage_rate() + 1000)
    end,

    ON_UNBUFF = function(me, sp)
        me:damage_rate(me:damage_rate() - 1000)
    end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
