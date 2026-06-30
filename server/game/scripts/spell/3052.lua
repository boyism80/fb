-- spell: 마비
-- 마비 캐스팅
local spell = require('lib.spell')

function ON_CAST_3052(me, you, sp)
    local mp = 80
    local sound = 70
    local effect = 1
    local buff_time = 20
    if spell.debuff_cast(me, you, sp, {mp = mp, sound = sound, effect = effect}) then
        you:buff(sp, buff_time, me)
    end
end

-- 마비 버프 효과
function ON_BUFF_3052(me, sp)
    me:paralysis(true)
end

-- 마비 버프 해제 효과
function ON_UNBUFF_3052(me, sp)

    me:paralysis(false)
end
