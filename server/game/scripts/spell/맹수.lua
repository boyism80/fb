-- 맹수 캐스팅
local spell = require('lib.spell')

function ON_CAST_2007(me, sp, name)
    local mobs = {
        ['곰'] = name2mob('곰'):look(),
        ['호랑이'] = name2mob('호랑이'):look(),
    }

    local mp = 30
    local effect = 3
    local sound = 25
    local buff_time = 60
    spell.disguise(me, mobs, name, sp, {mp=mp, sound=sound, effect=effect, buff_time=buff_time})
end

-- 맹수 버프 효과
function ON_BUFF_2007(me, sp)

end

-- 맹수 버프 해제 효과
function ON_UNBUFF_2007(me, sp)
    me:mimic(nil)
end