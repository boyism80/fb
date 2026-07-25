-- spell: 분혼경천
-- 분혼경천 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = (me:hp()*3)//4
        local hp = me:hp()//2
        local mp = 200
        local sound = 508
        local effect = 160

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 7, 6), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={me=effect}})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
