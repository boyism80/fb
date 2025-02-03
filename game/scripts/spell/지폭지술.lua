function on_cast(me, spell)
    local damage = (me:mp()*3)//2
    local hp = nil
    local mp = math.max(me:mp(), 30)
    local sound = 107
    local effect = 104

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 8, 7), spell, damage, hp, mp, sound, nil, effect)
end