-- 필살검무 캐스팅
function ON_CAST_2016(me, spell)
    local effect = 7
    local sound = 35
    local hp = me:hp()//2
    local mp = math.max(me:mp(), 60)
    local damage = me:hp() + me:mp()
    local message = '필살검무'

    local front = me:front(OBJECT_TYPE.LIFE)
    if not attack_cast(me, front, spell, {hp = hp, mp = mp, damage = damage, message = message, sound = sound, effect = effect}) then
        return
    end
end