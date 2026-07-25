-- spell: 포효검황
-- 포효검황 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = (me:hp()*11)//10
        local hp = me:hp()//2
        local mp = 600
        local sound = 101
        local effect = 85

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 8, 7), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
