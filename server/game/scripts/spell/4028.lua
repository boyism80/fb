-- spell: 동해의기원
-- 동해의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4028(me, you, sp)
    local hp = 100
    local mp = 30
    local sound = 5
    local effect = 5
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end
