-- item: 투명구두

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    ON_CONCAST = function(me, item)
        me:cast('자동투명')
    end,

    -- ON_ATTACK = function(me, item)
    -- end
}
