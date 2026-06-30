-- spell: 필살검무
-- 필살검무 캐스팅
local spell = require('lib.spell')

function ON_CAST_2016(me, sp)
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
end
