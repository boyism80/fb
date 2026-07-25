-- item: 빨간시약

return {
    ON_ACTIVATED = function(me, item)
        local value = me:maxhp() // 3
        me:hp(math.min(me:hp()+value, me:maxhp()))
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
