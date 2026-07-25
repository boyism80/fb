-- item: 용천제구검

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    on_attack = function(me, item)
        math.randomseed(seed())
        if math.random() > 0.50 then
            return false
        end
        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            return
        end
        me:cast(front, '용천제구격')
    end
}
