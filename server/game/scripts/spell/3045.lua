-- spell: 지폭지술
-- 지폭지술 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local damage = (me:mp()*3)//2
        local hp = nil
        local mp = math.max(me:mp(), 30)
        local sound = 107
        local effect = 104

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 8, 7), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={you=effect}})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
