function on_cast(me, spell)
    local damage = (me:hp()*11)//10
    local hp = me:hp()//2
    local mp = 600
    local sound = 101
    local effect = 85

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 8, 7), spell, damage, hp, mp, sound, effect)
end