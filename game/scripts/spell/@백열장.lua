function handle_spell(me, spell, you)
	if you:isbuff('�鿭��') then
		me:message('�̹� �ɷ� �ֽ��ϴ�.')
		return
	end

	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)
	me:buff(spell, 5)
	me:message('�鿭���� �ܿ����ϴ�.')
	if(me ~= you) then
		you:message(string.format('%s���� �鿭���� �ܿ��ּ̽��ϴ�.', me:name()))
	end
end