-- 동귀어진 캐스팅
function ON_CAST_1015(me, spell)
    local effect = 9
    local sound = 18
    local hp = me:hp() - 10
    local mp = 10
    local damage = me:hp()*2
    local message = '동귀어진'

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
        return
    end
end