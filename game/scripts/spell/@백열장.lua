function handle_spell(me, spell, you)
	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)
end