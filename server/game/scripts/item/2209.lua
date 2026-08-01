-- item: 진백세주

return {
    on_activated = function(me, item)
        me:mp(math.min(me:mp()+150, me:maxmp()))
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
