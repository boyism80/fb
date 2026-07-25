-- item: 빨간시약

return {
    on_activated = function(me, item)
        local value = me:maxhp() // 3
        me:hp(math.min(me:hp()+value, me:maxhp()))
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
