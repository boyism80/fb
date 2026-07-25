-- item: 맑은시약

return {
    ON_ACTIVATED = function(me, item)
        me:hp(me:maxhp())
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
