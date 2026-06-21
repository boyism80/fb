-- 일성금강퇴: deal damage to the target in front; consumes caster HP.
local spell = require('lib.spell')

function ON_CAST_114(me, sp)
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        me:message('대상이 없습니다.')
        return
    end
    local hp_cost = 240
    local damage = 240
    if me:hp() < hp_cost then
        me:message('체력이 부족합니다.')
        return
    end
    me:hp(me:hp() - hp_cost)
    spell.damage(me, front, sp, { damage = damage, mp = 0, sound = 8, effect = 32 })
end
