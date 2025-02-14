function on_cast(me, spell)
    local damage = (me:hp()*3)//4
    local hp = me:hp()//2
    local mp = 600
    local sound = 507
    local effect = 162

    spell_damage_area(me, me:nears(OBJECT_TYPE_LIFE, 6, 5), spell, damage, hp, mp, sound, effect)
end