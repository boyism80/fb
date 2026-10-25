-- item: 신국내성비서

local server = require('lib.server')

return {
    on_activated = function(me, item)
        local maps = {
            '신국내북서주막',
            '신국내북주막',
            '신국내북동주막',
            '신국내남서주막',
            '신국내남주막',
            '신국내남동주막',
        }

        math.randomseed(seed())
        local i = math.random(1, #maps)
        local map = maps[i]

        me:map(map, nil, server.item_cost('scripts/item/60029.lua', item:model():name()))
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
