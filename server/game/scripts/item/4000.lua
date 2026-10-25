-- item: 노란비서

-- Yellow Note use script

local server = require('lib.server')

return {
    on_activated = function(me, item)
        server.warp_to_return_map(me, server.item_cost('scripts/item/4000.lua', item:model():name()))
    end,

    on_before_warp = function(me, name)
        return server.pay_item(me, name)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
