-- spell: 저주
-- 저주 캐스팅
local spell = require('lib.spell')

function ON_CAST_3051(me, you, sp)
    local mp = 30
    local sound = 69
    local effect = 13
    local buff_time = 185
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 저주 버프 효과
function ON_BUFF_3051(me, sp)
    me:buff_phydef(me:buff_phydef() + 30)
end

-- 저주 버프 해제 효과
function ON_UNBUFF_3051(me, sp)
    me:buff_phydef(me:buff_phydef() - 30)
end
