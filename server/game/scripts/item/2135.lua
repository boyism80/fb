-- item: 보무의목걸이

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    ON_CONCAST = function(me, item)
        me:cast('자동무장')
        me:cast('자동보호')
    end,

    -- ON_ATTACK = function(me, item)
    -- end
}
