-- item: 막걸리

return {
    ON_ACTIVATED = function(me, item)
        me:hp(math.max(1, me:hp()-1), false)
        me:mp(math.min(me:mp()+30, me:maxmp()), false)
        me:update()
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
