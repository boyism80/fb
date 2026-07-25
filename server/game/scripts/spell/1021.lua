-- spell: 혈겁만파
-- 혈겁만파 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = (me:hp()*3)//4
        local hp = me:hp()//2
        local mp = 600
        local sound = 507
        local effect = 162

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 6, 5), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
