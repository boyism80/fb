-- item: 소환비서

return {
    on_activated = function(me, item)
        local front = me:front(OBJECT_TYPE.CHARACTER)
        if front ~= nil then
            if front:level() < me:level() then
                local x, y = me:position()
                front:position(x, y)
            end
        end

        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
