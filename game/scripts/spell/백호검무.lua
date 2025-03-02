-- 백호검무 캐스팅
function ON_CAST_2027(me, spell)
    local effect = 69
    local sound = 90
    local hp = me:hp()//2
    local mp = 60
    local damage = me:hp() + math.min(me:mp(), 20000)

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '백호검무', sound, effect) then
        return
    end
end