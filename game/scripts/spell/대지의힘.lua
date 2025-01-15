function on_cast(me, you, spell)
	if not you:is(OBJECT_TYPE_CHARACTER) then
		return me:message('걸리지 않습니다.')
	end

    local mp = 30
    local sound = 21
    local effect = 11
    local buff_time = 5
    if buff_cast(me, you, spell, mp, sound, effect) then
        you:buff_str(you:buff_str()+3)
        you:buff(spell, buff_time)
    end
end

function on_uncast(me, spell)
    me:buff_str(me:buff_str()-3)
    unbuff(me, spell)
end