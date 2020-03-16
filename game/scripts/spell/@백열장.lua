function handle_spell(me, spell, you)
	you:direction((you:direction() + 1) % 4)
end