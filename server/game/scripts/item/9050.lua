-- item: 의태시약

local spell = require('lib.spell')

return {
    ON_ACTIVATED = function(me, item)
        local front = spell.front_exclude_item(me)
        if front == nil then
            return
        end

        me:cast(front, '의태')
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
