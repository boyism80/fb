-- item: 마법의시약

return {
    on_activated = function(me, item)
        me:mp(me:maxmp())
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
