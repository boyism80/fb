function on_cast(me, you, spell)
	-- TODO: 무기에 따른 딜레이

    local damage = math.floor(me:mp() * 1.5)
    local mp = me:mp()
    local sound = 73
    local effect = 8
    spell_damage(me, you, spell, damage, mp, sound, effect)
end