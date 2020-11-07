function handle_spell(me, spell, you)
	you:effect(0x89)
	me:action(0x03)
	you:sound(0x08)
	me:chat('청룡마령참')
end