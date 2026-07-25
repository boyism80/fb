-- item: 청의태자변신시약

return {
    ON_ACTIVATED = function(me, item)
        local look = name2mob('청의태자'):look()
        me:mimic({ disguise = look })
        me:effect(3)
        me:sound(25)
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
