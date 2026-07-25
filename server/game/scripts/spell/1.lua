-- spell: 비영사천문
-- 비영사천문 캐스팅
local spell = require('lib.spell')

return {
    ON_CAST = function(me, sp, message)
        local map = me:map()
        if map == nil then
            return
        end

        local error = me:assert(STATE.GHOST, STATE.RIDING)
        if error ~= nil then
            me:message(error)
            return
        end

        local input = nil
        if message == '동' or message == '1' then
            input = CARDINAL_DIRECTION.EAST
        elseif message == '서' or message == '2' then
            input = CARDINAL_DIRECTION.WEST
        elseif message == '남' or message == '3' then
            input = CARDINAL_DIRECTION.SOUTH
        elseif message == '북' or message == '4' then
            input = CARDINAL_DIRECTION.NORTH
        else
            return
        end

        local mp = 30
        local effect = nil
        local sound = 1
        if spell.cast(me, me, sp, {mp=mp, sound=sound, effect=effect}) then
            local root = map:model():root()
            local cardinal = root:cardinal(input)
            if cardinal == nil then
                return
            end

            local dest, left, top, right, bottom = table.unpack(cardinal)
            math.randomseed(seed())
            local x = math.random(left, right)
            local y = math.random(top, bottom)
            me:map(dest:name(), x, y)
            me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
        end
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
