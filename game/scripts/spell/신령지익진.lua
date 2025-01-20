function on_uncast(me, spell)
	me:damage_derate(me:damage_derate() - 1000)
end

function on_concast(me, caster, buff)

end