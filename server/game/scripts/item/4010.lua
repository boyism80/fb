-- item: 도삭산100층비서

local server = require('lib.server')

return {
    on_activated = function(me, item)
        me:map('도삭산100층주막', nil, server.item_cost('scripts/item/4010.lua', item:model():name()))
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
