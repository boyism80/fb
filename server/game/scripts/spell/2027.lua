-- spell: 백호검무
-- 백호검무 캐스팅
local spell = require('lib.spell')

function ON_CAST_2027(me, sp)
    local effect = 69
    local sound = 90
    local hp = me:hp()//2
    local mp = 60
    local damage = me:hp() + math.min(me:mp(), 20000)
    local message = '백호검무'

    local front = me:front(OBJECT_TYPE.LIFE)
    if not spell.attack_cast(me, front, sp, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
        return
    end
end
