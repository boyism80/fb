-- item: 사랑호출기

local spell = require('lib.spell')

return {
    on_activated = function(me, item)
        local m = me:marriage()
        if not m.married then
            return me:message('결혼하지 않아 사용할 수 없습니다.')
        end

        local spouse = id2ch(m.spouse_id)
        if spouse == nil then
            return me:message(string.format('%s님은 현재 접속 중이 아닙니다.', m.spouse_name))
        end

        local map = spouse:map()
        if map == nil then
            return me:message('이동할 수 없는 공간입니다.')
        end

        local x, y = spouse:position()
        local direction = me:direction()
        local new_x, new_y, direction = spell.TELEPORT_LOOKUP(me, map, x, y, direction)
        me:map(map, new_x, new_y)
        me:direction(direction)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
