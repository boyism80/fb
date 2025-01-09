function on_cast(me, spell, name)

	local npc = name2npc('낙랑')
	local ch = name2ch(name)
	if ch == nil then
		return
	end

	local map = ch:map()
	if map == nil then
		npc:dialog(me, 'map is nil')
		return
	end

	math.randomseed(seed())
	local offset = math.random(0, 3)
	local x, y = ch:position()
	local rand_x = 0
	local rand_y = 0
	local direction = nil
	for i = 0, 3 do
		local case = (offset + i) % 4
		if case == 0 then
			rand_x = 0
			rand_y = -1
			direction = DIRECTION_BOTTOM
		elseif case == 1 then
			rand_x = 0
			rand_y = 1
			direction = DIRECTION_TOP
		elseif case == 2 then
			rand_x = -1
			rand_y = 0
			direction = DIRECTION_RIGHT
		else
			rand_x = 1
			rand_y = 0
			direction = DIRECTION_LEFT
		end

		if map:movable(x+rand_x, y+rand_y) then
			break
		end
	end

	me:map(map, x+rand_x, y+rand_y)
	me:direction(direction)
	me:action(ACTION_CAST_SPELL, DURATION_SPELL, 29)
	me:effect(16)
end