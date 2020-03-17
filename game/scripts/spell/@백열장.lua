function handle_spell(me, spell, you)
	-- you:direction((you:direction() + 1) % 4)

	for i, object in pairs(me:showings(0x04)) do
		me:damage(object, 100)
	end
end