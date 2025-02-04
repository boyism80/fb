function on_cast(me, spell, message)
    local mp = 60
    local dam = 10
    spell_weapon_damage(me, mp, message, dam)
end