-- item: 팔과탕

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp()+1000, me:maxhp()), false)
        me:mp(math.min(me:mp()+1000, me:maxmp()), false)
        me:update()
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
