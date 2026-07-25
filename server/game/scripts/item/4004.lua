-- item: 국내성비서

return {
    on_activated = function(me, item)
    	local maps = {'주막왈숙네', '왈숙언니네', '왈숙이모네'}

        math.randomseed(seed())
        local i = math.random(1, #maps)
        local map = maps[i]

        if me:map(map) then
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
