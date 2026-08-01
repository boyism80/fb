-- item: 맑은시약

return {
    on_activated = function(me, item)
        me:hp(me:maxhp())
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
