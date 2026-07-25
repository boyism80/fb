-- spell: 혈겁만파
-- 혈겁만파 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = (me:hp()*3)//4
        local hp = me:hp()//2
        local mp = 600
        local sound = 507
        local effect = 162

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 6, 5), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
