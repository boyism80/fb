-- spell: 화영열주'첨
-- 화영열주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3026(me, sp)
    local damage = 510
    local mp = 250
    local sound = 41
    local effect = 46
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
