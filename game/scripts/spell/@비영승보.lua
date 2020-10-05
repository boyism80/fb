-- TOP = 0x00, RIGHT = 0x01, BOTTOM = 0x02, LEFT = 0x03

function lookup(me, you, direction)
	local map = me:map()
	
	local mine_x, mine_y = me:position()
	local your_x, your_y = you:position()
	if direction == 0x00 then
		mine_x = your_x
		mine_y = your_y - 1
	elseif direction == 0x01 then
		mine_x = your_x + 1
		mine_y = your_y
	elseif direction == 0x02 then
		mine_x = your_x
		mine_y = your_y + 1
	else
		mine_x = your_x - 1
		mine_y = your_y
	end
	
	if map:movable(mine_x, mine_y) == false then
		return false
	end

	local newdir = (direction + 2) % 0x04
	me:position(mine_x, mine_y)
	me:direction(newdir)
	return true
end


function handle_spell(me, spell)
	local front = me:front(0xff & (~0x01))
	if front == nil then
		return
	end

	local direction = me:direction()
	if lookup(me, front, (direction + 0) % 4) or 
	   lookup(me, front, (direction + 3) % 4) or
	   lookup(me, front, (direction + 1) % 4) then

		if me:isbuff('투명') and front:is(0x04 | 0x08) then
			me:unbuff('투명')
		end
		me:action(0x01, 0x0a, 0x1e)
		me:message('비영승보를 외웠습니다.')
	end
end