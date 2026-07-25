-- item: 용천제일검

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    ON_ATTACK = function(me, item)
        math.randomseed(seed())
        if math.random() > 0.20 then
            return false
        end
        local front = me:front(OBJECT_TYPE.LIFE)
        if front == nil then
            return
        end
        me:cast(front, '용천제일격')
    end
}
