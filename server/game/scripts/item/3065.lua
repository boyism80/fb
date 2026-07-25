-- item: 산해진미

return {
    on_activated = function(me, item)
        me:base_hp(me:base_hp() + 200)
        me:base_mp(me:base_mp() + 100)
    end,

    -- on_deactivated = function(me, item)
    -- end,

    -- on_concast = function(me, item)
    -- end,

    -- on_attack = function(me, item)
    -- end
}
