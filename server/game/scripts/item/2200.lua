-- item: 동동주

return {
    on_activated = function(me, item)
        me:hp(math.max(1, me:hp()-1), false)
        me:mp(math.min(me:mp()+30, me:maxmp()), false)
        me:update()
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
