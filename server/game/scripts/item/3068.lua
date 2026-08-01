-- item: 실패한요리

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp()+5, me:maxhp()))
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
