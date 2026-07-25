-- item: 홍주

return {
    ON_ACTIVATED = function(me, item)
        me:mp(math.min(me:mp()+300, me:maxmp()))
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
