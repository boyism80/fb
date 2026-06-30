-- spell: 삼매진화
-- 삼매진화 캐스팅
local spell = require('lib.spell')

function ON_CAST_3044(me, you, sp)
    local damage = math.floor(me:mp() * 1.5)
    local mp = me:mp()
    local sound = 73
    local effect = 8
    spell.damage_near_target(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
end
