-- spell: 해독
-- 해독 캐스팅
local spell = require('lib.spell')

function ON_CAST_4023(me, you, sp)
    local mp = 30
    local sound = 63
    local effect = 22
    if spell.cast(me, you, sp, {mp=mp, sound=sound, effect=effect}) then
        you:unbuff('중독')
    end
end
