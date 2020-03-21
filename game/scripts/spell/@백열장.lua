function handle_spell(me, spell, you)
	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)

	me:buff(spell, 5)
	if(me == you) then
		me:message('백열장을 외웠습니다.')
	else
		you:message(string.format('%s님이 백열장을 외워주셨습니다.', me:name()))
	end
end