-- item: 나비채
local jar = require('lib.butterfly_jar')

local MAP_DALMAJI = 10946
local MOB_BUTTERFLY_MIN = 1335
local MOB_BUTTERFLY_MAX = 1343

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    on_attack = function(me, item)
        local map = me:map()
        if map == nil or map:model():id() ~= MAP_DALMAJI then
            return
        end

        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil or not front:is(OBJECT_TYPE.MOB) then
            return
        end

        local mob_id = front:model():id()
        if mob_id < MOB_BUTTERFLY_MIN or mob_id > MOB_BUTTERFLY_MAX then
            return
        end

        math.randomseed(seed())
        if math.random(1, 5) > 1 then
            me:message("나비가 날아갔다!")
            return
        end

        jar.upgrade(me)
    end
}
