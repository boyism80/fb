-- item: 의태시약

local spell = require('lib.spell')

return {
    on_activated = function(me, item)
        local front = spell.front_exclude_item(me)
        if front == nil then
            return
        end

        me:cast(front, '의태')
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
