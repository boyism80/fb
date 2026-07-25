-- item: 파란열쇠

return {
    ON_ACTIVATED = function(me, item)
        local map = me:map()
        local door = map:door(me)
        if door == nil then
            return
        end

        local locked = door:lock(not door:locked())
        if locked then
            me:message('문을 잠궜습니다.')
        else
            me:message('문을 열었습니다.')
        end
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
