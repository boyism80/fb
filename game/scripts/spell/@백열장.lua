function handle_spell(me, spell, you)
	me:action(0x06)
	me:state(0x05)
	me:effect(0x14)
	me:sound(0x001C)
end