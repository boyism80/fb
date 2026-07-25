-- item: 토깽이변신시약

return {
    ON_ACTIVATED = function(me, item)
        local look = name2mob('토깽이'):look()
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
