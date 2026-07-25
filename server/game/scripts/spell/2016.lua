-- spell: 필살검무
-- 필살검무 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp)
        local effect = 7
        local sound = 35
        local hp = me:hp()//2
        local mp = math.max(me:mp(), 60)
        local damage = me:hp() + me:mp()
        local message = '필살검무'

        local front = me:front(OBJECT_TYPE.LIFE)
        if not spell.attack_cast(me, front, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
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
