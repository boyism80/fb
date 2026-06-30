-- spell: 활력
-- 활력 캐스팅
local spell = require('lib.spell')

function ON_CAST_4024(me, you, sp)
    local mp = 30
    local sound = 63
    local effect = 22
    if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('마비')
    end
end
