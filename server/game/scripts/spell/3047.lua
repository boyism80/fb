-- spell: 현자의기원
-- callfunc "TARGETHEALMAGIC", 1000, 5000, 168, 503, "현자의기원"
-- 현자의기원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local hp = 5000
        local mp = 1000
        local sound = 503
        local effect = 168
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
