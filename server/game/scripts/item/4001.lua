-- item: 소환비서

return {
    ON_ACTIVATED = function(me, item)
        local front = me:front(OBJECT_TYPE.CHARACTER)
        if front ~= nil then
            if front:level() < me:level() then
                local x, y = me:position()
                front:position(x, y)
            end
        end

        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
