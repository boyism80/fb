function callback(ch)
	if ch == nil then
		return nil, nil
	end

	return ch:map(), {ch:position()}
end

function on_cast(me, spell, name)

	local map, position = name2ch(name, callback)
	local npc = name2npc('낙랑')

	if map == nil then
		npc:dialog(me, 'map is nil')
		return
	end

	me:map(map, position)
end