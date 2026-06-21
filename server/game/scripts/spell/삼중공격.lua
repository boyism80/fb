-- 삼중공격 캐스팅
local spell = require('lib.spell')

function ON_CAST_1010(me, sp)
    local mp = 60
    local sound = 9
    local effect = 71
    local buff_time = 600

    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time)
    end
end

-- 삼중공격 버프 효과
function ON_BUFF_1010(me, sp)
    me:unbuff('이중공격')
    me:unbuff('사중공격')
    me:unbuff('오중공격')
    me:damage_rate(me:damage_rate() + 2000)
end

-- 삼중공격 버프 해제 효과
function ON_UNBUFF_1010(me, sp)
    me:damage_rate(me:damage_rate() - 2000)

end