-- spell: 이기어검
-- 이기어검 캐스팅
local spell = require('lib.spell')

return {
    on_cast = function(me, sp)
        local function preprocess(me, you)
            you:buff('이기어검술', 2, me)
        end

        local effect = 68
        local sound = 91
        local hp = me:hp() // 2
        local mp = 180
        local damage = me:hp() + math.min(20000, me:mp())
        local message = '이기어검'

        local front = me:front(OBJECT_TYPE.LIFE)
        if not spell.attack_cast(me, front, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect, preprocess = preprocess}) then
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
