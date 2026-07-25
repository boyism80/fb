-- item: 파란열쇠

return {
    on_activated = function(me, item)
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

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
