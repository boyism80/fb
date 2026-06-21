-- 무장 캐스팅
local spell = require('lib.spell')

function ON_CAST_4003(me, you, sp)
    local mp = 30
    local sound = 8
    local effect = 110
    local buff_time = 180
    if spell.buff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 무장 버프 효과
function ON_BUFF_4003(me, sp)
    me:buff_phydef(me:buff_phydef() -10)
end

-- 무장 버프 해제 효과
function ON_UNBUFF_4003(me, sp)
    me:buff_phydef(me:buff_phydef() + 10)
end