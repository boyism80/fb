function on_uncast(me, spell)
	me:damage_rate(me:damage_rate() - 1000)
    me:skill_damage_rate(me:skill_damage_rate() - 1000)
end

function on_concast(me, caster, buff)

end