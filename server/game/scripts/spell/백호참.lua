-- 백호참 캐스팅
function ON_CAST_1016(me, spell)
    local effect = 67
    local sound = 87
    local hp = me:hp() // 2
    local mp = 60
    local damage = me:hp()
    local message = '백호참'

    local front = me:front(OBJECT_TYPE.LIFE)
    if not attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
        return
    end
end