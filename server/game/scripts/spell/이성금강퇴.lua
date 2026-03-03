-- 이성금강퇴: deal damage to the target in front; consumes caster HP.
function ON_CAST_115(me, spell)
    local front = me:front(OBJECT_TYPE.LIFE)
    if front == nil then
        me:message('대상이 없습니다.')
        return
    end
    local hp_cost = 480
    local damage = 480
    if me:hp() < hp_cost then
        me:message('체력이 부족합니다.')
        return
    end
    me:hp(me:hp() - hp_cost)
    spell_damage(me, front, spell, { damage = damage, mp = 0, sound = 8, effect = 32 })
end
