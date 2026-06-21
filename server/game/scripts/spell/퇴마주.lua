-- 퇴마주 캐스팅
local spell = require('lib.spell')

function ON_CAST_4022(me, you, sp)
    local mp = 30
    local sound = 67
    local effect = 22
    if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('저주')
    end
end