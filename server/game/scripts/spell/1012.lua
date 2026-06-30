-- spell: 오중공격
-- 오중공격 캐스팅
local spell = require('lib.spell')

function ON_CAST_1012(me, sp)
    local mp = 200
    local sound = 9
    local effect = 72
    local buff_time = 600

    if spell.buff_cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
        me:buff(sp, buff_time)
    end
end

-- 오중공격 버프 효과
function ON_BUFF_1012(me, sp)
    me:unbuff('이중공격')
    me:unbuff('삼중공격')
    me:unbuff('사중공격')
    me:damage_rate(me:damage_rate() + 4000)
end

-- 오중공격 버프 해제 효과
function ON_UNBUFF_1012(me, sp)
    me:damage_rate(me:damage_rate() - 4000)

end
