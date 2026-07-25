-- item: 용마제이검

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    on_attack = function(me, item)
        math.randomseed(seed())
        if math.random() > 0.25 then
            return false
        end
        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            return
        end
        me:cast(front, '용마제이격')
    end
}
