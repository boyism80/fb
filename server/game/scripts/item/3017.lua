-- item: 돼지고기

return {
    on_activated = function(me, item)
        me:hp(math.min(me:hp()+62, me:maxhp()))
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
