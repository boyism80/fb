-- spell: 지폭지술
-- 지폭지술 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local damage = (me:mp()*3)//2
        local hp = nil
        local mp = math.max(me:mp(), 30)
        local sound = 107
        local effect = 104

        spell.damage_area(me, me:nears(OBJECT_TYPE.LIFE, 8, 7), sp, {damage=damage, hp=hp, mp=mp, sound=sound, effect={you=effect}})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
