-- spell: 혼마술
-- 혼마술 캐스팅
local spell = require('lib.spell')

function ON_CAST_4009(me, you, sp)
    local mp = 40
    local sound = 79
    local effect = 39
    local buff_time = 425
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 혼마술 버프 효과
function ON_BUFF_4009(me, sp)
    me:buff_phydef(me:buff_phydef() + 50)
end

-- 혼마술 버프 해제 효과
function ON_UNBUFF_4009(me, sp)
    me:buff_phydef(me:buff_phydef() - 50)
end
