function handle_spell(me, spell)
	if me:isbuff(spell) then
		me:message('�̹� �ɷ� �ֽ��ϴ�.')
		return
	end

	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)
	me:buff(spell, 5)
	me:message('������ �ܿ����ϴ�.')
end