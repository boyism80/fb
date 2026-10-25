-- item: 신부여성비서

local server = require('lib.server')

return {
    on_activated = function(me, item)
        local maps = { '신부여성주막', '신부여서쪽주막', '신부여서쪽주막2' }

        math.randomseed(seed())
        local i = math.random(1, #maps)
        local map = maps[i]

        me:map(map, nil, server.item_cost('scripts/item/4025.lua', item:model():name()))
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
