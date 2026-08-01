-- spell: 천공의희원
-- 천공의희원 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, you, sp)
        local hp = 200
        local mp = 200
        local sound = 34
        local effect = 66
        spell.heal(me, you, sp, {hp=hp, mp=mp, sound=sound, effect=effect})
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
