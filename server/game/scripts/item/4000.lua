-- item: 노란비서

-- Yellow Note use script

local server = require('lib.server')

return {
    ON_ACTIVATED = function(me, item)
        if server.warp_to_return_map(me) then
            me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
        end
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
