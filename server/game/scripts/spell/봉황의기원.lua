-- 봉황의기원 캐스팅
local spell = require('lib.spell')

function ON_CAST_4041(me, you, sp)
    local hp = 30000
    local mp = 10000
    local sound = 505
    local effect = 165
    spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
end