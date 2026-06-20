-- 자영무주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3027(me, sp)
    local damage = 510
    local mp = 250
    local sound = 46
    local effect = 24
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end