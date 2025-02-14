function on_cast(me, spell)
    local effect = 7
    local sound = 35
    local hp = me:hp()//2
    local mp = math.max(me:mp(), 60)
    local damage = me:hp() + me:mp()

    local front = me:front(OBJECT_TYPE_LIFE)
    if not attack_cast(me, front, spell, hp, mp, damage, '필살검무', sound, effect) then
        return
    end
end