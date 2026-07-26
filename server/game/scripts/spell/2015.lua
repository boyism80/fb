-- spell: 비영승보
local spell = require('lib.spell')

-- 비영승보 캐스팅
local function on_cast_lookup(me, you, direction)
    local map = me:map()

    local mine_x, mine_y = me:position()
    local your_x, your_y = you:position()
    if direction == DIRECTION.TOP then
        mine_x = your_x
        mine_y = your_y - 1
    elseif direction == DIRECTION.RIGHT then
        mine_x = your_x + 1
        mine_y = your_y
    elseif direction == DIRECTION.BOTTOM then
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

return {
    on_cast = function(me, sp)
        local err = me:assert({STATE.GHOST, STATE.RIDING})
        if err then
            me:message(err)
            return
        end

        local front = spell.front_exclude_item(me)
        if front == nil then
            return
        end

        local direction = me:direction()
        for _, dir in pairs({direction + 0, direction + 3, direction + 1}) do
            if on_cast_lookup(me, front, dir % 4) then
                me:attack(14)
                me:action(ACTION.CAST_SPELL, 25)
                me:message('비영승보를 외웠습니다.')
                me:sound(30)
                break
            end
        end
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
