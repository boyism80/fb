-- spell: 삼매진화
-- 삼매진화 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, you, sp)
        local damage = math.floor(me:mp() * 1.5)
        local mp = me:mp()
        local sound = 73
        local effect = 8
        spell.damage_near_target(me, you, sp, {damage=damage, mp=mp, sound=sound, effect=effect})
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
