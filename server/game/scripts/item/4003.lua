-- item: 부여성비서

local server = require('lib.server')

return {
    on_activated = function(me, item)
        local maps = { '주막연실이네', '주막연실언니네', '주막연실이모네' }

        math.randomseed(seed())
        local i = math.random(1, #maps)
        local map = maps[i]

        me:map(map, nil, server.item_cost('scripts/item/4003.lua', item:model():name()))
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
