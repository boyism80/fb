function handle_spell(me, spell, you)
	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)

	me:buff(spell, 5)
	if(me == you) then
		me:message('�鿭���� �ܿ����ϴ�.')
	else
		you:message(string.format('%s���� �鿭���� �ܿ��ּ̽��ϴ�.', me:name()))
	end
end