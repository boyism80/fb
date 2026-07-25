-- item: 노란비서

-- Yellow Note use script

local server = require('lib.server')

return {
    on_activated = function(me, item)
        if server.warp_to_return_map(me) then
            me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
        end
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
