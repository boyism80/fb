-- item: 마법의시약

return {
    ON_ACTIVATED = function(me, item)
        me:mp(me:maxmp())
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
