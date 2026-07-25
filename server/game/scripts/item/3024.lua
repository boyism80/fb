-- item: 호랑이가죽

return {
    ON_ACTIVATED = function(me, item)
        me:hp(math.min(me:hp()+18, me:maxhp()))
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
