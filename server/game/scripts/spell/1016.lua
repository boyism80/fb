-- spell: 백호참
-- 백호참 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local effect = 67
        local sound = 87
        local hp = me:hp() // 2
        local mp = 60
        local damage = me:hp()
        local message = '백호참'

        local front = me:front(OBJECT_TYPE.LIFE)
        if not spell.attack_cast(me, front, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
            return
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
