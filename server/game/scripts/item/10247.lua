-- item: 자양강장요리

return {
    ON_ACTIVATED = function(me, item)
        me:base_hp(me:base_hp() + 100)
        me:base_mp(me:base_mp() + 50)
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
