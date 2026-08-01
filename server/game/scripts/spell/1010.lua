-- spell: 삼중공격
-- 삼중공격 캐스팅
local spell = require('lib.spell')

-- 삼중공격 버프 효과

-- 삼중공격 버프 해제 효과

return {
    on_cast = function(me, sp)
        local mp = 60
        local sound = 9
        local effect = 71
        local buff_time = 600

        if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            me:buff(sp, buff_time)
        end
    end,

    on_buff = function(me, sp)
        me:unbuff('이중공격')
        me:unbuff('사중공격')
        me:unbuff('오중공격')
        me:damage_rate(me:damage_rate() + 2000)
    end,

    on_unbuff = function(me, sp)
        me:damage_rate(me:damage_rate() - 2000)
    end,

    -- on_concast = function(me, sp)
    -- end
}
