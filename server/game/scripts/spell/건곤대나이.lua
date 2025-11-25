-- 건곤대나이 캐스팅
function ON_CAST_1014(me, spell)
    local effect = 6
    local sound = 14
    local hp = me:hp()*2 // 3
    local mp = 40
    local damage = me:hp()
    local message = '건곤대나이'

    local front = me:front(OBJECT_TYPE.LIFE)
    if not attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
        return
    end
end