-- 화염주'첨 캐스팅
local spell = require('lib.spell')

function ON_CAST_3022(me, sp)
	local damage = 300
    local mp = 180
    local sound = 40
    local effect = 45
    spell.damage_near(me, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end