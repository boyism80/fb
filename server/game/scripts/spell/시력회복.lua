-- 시력회복 캐스팅
local spell = require('lib.spell')

function ON_CAST_4006(me, you, sp)
    local mp = 40
    local sound = 75
    local effect = 10
    if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('절망')
    end
end