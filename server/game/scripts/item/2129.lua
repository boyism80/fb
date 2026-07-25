-- item: 해독의귀걸이

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    on_concast = function(me, item)
        if me:isbuff('중독') then
            me:unbuff('중독')
        end
    end,

    -- on_attack = function(me, item)
    -- end
}
