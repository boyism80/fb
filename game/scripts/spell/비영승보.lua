function ON_CAST_2015_LOOKUP(me, you, direction)
    local map = me:map()
    
    local mine_x, mine_y = me:position()
    local your_x, your_y = you:position()
    if direction == DIRECTION_TOP then
        mine_x = your_x
        mine_y = your_y - 1
    elseif direction == DIRECTION_RIGHT then
        mine_x = your_x + 1
        mine_y = your_y
    elseif direction == DIRECTION_BOTTOM then
        mine_x = your_x
        mine_y = your_y + 1
    else
        mine_x = your_x - 1
        mine_y = your_y
    end
    
    if map:movable(me, mine_x, mine_y) == false then
        return false
    end

    local newdir = (direction + 2) % 0x04
    me:position(mine_x, mine_y)
    me:direction(newdir)
    return true
end

-- 비영승보 캐스팅
function ON_CAST_2015(me, spell)
    local err = me:assert({STATE_GHOST, STATE_RIDING})
    if err then
        me:message(err)
        return
    end

    local front = me:front(0xff & (~OBJECT_TYPE_ITEM))
    if front == nil then
        return
    end

    local direction = me:direction()
    for _, dir in pairs({direction + 0, direction + 3, direction + 1}) do
        if ON_CAST_2015_LOOKUP(me, front, dir % 4) then
            me:attack(14)
            me:action(ACTION_CAST_SPELL, 25)
            me:message('비영승보를 외웠습니다.')
            me:sound(30)
            break
        end
    end
end