-- item: 자양강장요리

return {
    on_activated = function(me, item)
        me:base_hp(me:base_hp() + 100)
        me:base_mp(me:base_mp() + 50)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
