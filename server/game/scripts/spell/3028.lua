-- spell: 백령주'첨
-- 백령주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3028(me, sp)
    local damage = 510
    local mp = 250
    local sound = 51
    local effect = 41
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
