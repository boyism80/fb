function on_cast(me, spell)
    local effect = 9
    local sound = 18
    local hp = me:hp() - 10
    local mp = 10
    local damage = me:hp()*2

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '동귀어진', sound, effect) then
        return
    end
end