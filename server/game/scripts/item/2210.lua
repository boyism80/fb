-- item: 청심사주

return {
    ON_ACTIVATED = function(me, item)
        me:hp(math.min(me:hp()+150, me:maxhp()), false)
        me:mp(math.min(me:mp()+100, me:maxmp()), false)
        me:update()
    end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    -- ON_CONCAST = function(me, item)
    -- end,

    -- ON_ATTACK = function(me, item)
    -- end
}
