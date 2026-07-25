-- item: 청의태자변신시약

return {
    on_activated = function(me, item)
        local look = name2mob('청의태자'):look()
        me:mimic({ disguise = look })
        me:effect(3)
        me:sound(25)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
