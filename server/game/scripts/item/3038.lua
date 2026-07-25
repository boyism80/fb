-- item: 산돼지고기

return {
    ON_ACTIVATED = function(me, item)
        me:hp(math.min(me:hp()+78, me:maxhp()))
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
