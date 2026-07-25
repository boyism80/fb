-- spell: 백호의희원'첨
-- 백호의희원'첨 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local hp = me:mp()*2
        local mp = me:mp()//2
        local sound = 95
        local effect = 64
        if not spell.heal_group(me, sp, {hp=hp, mp=mp, sound=sound, effect=effect}) then
            return
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
