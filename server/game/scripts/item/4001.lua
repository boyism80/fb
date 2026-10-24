-- item: 소환비서

return {
    on_activated = function(me, item)
        local front = me:front(OBJECT_TYPE.CHARACTER)
        if front == nil or front:level() >= me:level() then
            me:message('앞에 소환할 수 있는 사람이 없습니다.')
            return
        end

        local x, y = me:position()
        front:position(x, y)
        me:rmitem(item, 1, ITEM_DELETE_TYPE.REDUCE)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
