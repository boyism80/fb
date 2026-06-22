-- spell: 혈겁만파
-- 혈겁만파 캐스팅
local spell = require('lib.spell')

function ON_CAST_1021(me, sp)
    local damage = (me:hp()*3)//4
    local hp = me:hp()//2
    local mp = 600
    local sound = 507
    local effect = 162

    spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 6, 5), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
end
