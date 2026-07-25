-- item: 산해진미

return {
    ON_ACTIVATED = function(me, item)
        me:base_hp(me:base_hp() + 200)
        me:base_mp(me:base_mp() + 100)
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
