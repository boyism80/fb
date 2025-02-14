function on_cast(me, spell)
    local effect = 6
    local sound = 14
    local hp = me:hp()*2 // 3
    local mp = 40
    local damage = me:hp()

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '건곤대나이', sound, effect) then
        return
    end
end