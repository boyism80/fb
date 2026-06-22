-- spell: 포효검황
-- 포효검황 캐스팅
local spell = require('lib.spell')

function ON_CAST_1020(me, sp)
    local damage = (me:hp()*11)//10
    local hp = me:hp()//2
    local mp = 600
    local sound = 101
    local effect = 85

    spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 8, 7), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
end
