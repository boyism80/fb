function handle_spell(me, spell, message)
	local mob = name2mob(message)
	if mob == nil then
		return
	end

	me:disguise(mob:look())
	me:effect(0x03)
	me:sound(0x0019)
	me:buff(spell, 10)
end