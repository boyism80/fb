-- spell: 건곤대나이
-- 건곤대나이 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local effect = 6
        local sound = 14
        local hp = me:hp()*2 // 3
        local mp = 40
        local damage = me:hp()
        local message = '건곤대나이'

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
