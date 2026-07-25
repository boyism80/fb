-- item: 도삭산600층비서

return {
    ON_ACTIVATED = function(me, item)
        if me:map('도삭산600층주막') then
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
