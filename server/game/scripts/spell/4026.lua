-- spell: 바다의기원
-- 바다의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4026(me, sp)
    local hp = 100
    local mp = 50
    local sound = 3
    local effect = 5
    spell.heal(me, me, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end
