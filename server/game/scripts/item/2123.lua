-- item: 투명구두

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    on_concast = function(me, item)
        me:cast('자동투명')
    end,

    -- on_attack = function(me, item)
    -- end
}
