-- item: 보무의목걸이

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    on_concast = function(me, item)
        me:cast('자동무장')
        me:cast('자동보호')
    end,

    -- on_attack = function(me, item)
    -- end
}
