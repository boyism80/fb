function handle_spell(me, spell)
	if me:isbuff(spell) then
		me:message('이미 걸려 있습니다.')
		return
	end

	me:action(0x06, 0x00, 0x1c)
	me:state(0x05)
	me:effect(0x14)
	me:buff(spell, 5)
	me:message('투명을 외웠습니다.')
end