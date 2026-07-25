-- item: 시력회복의귀걸이

return {
    -- on_activated = function(me, item)
    -- end,

    -- on_deactivated = function(me, item)
    -- end,

    on_concast = function(me, item)
        if me:isbuff('절망') then
            me:unbuff('절망')
        end
    end,

    -- on_attack = function(me, item)
    -- end
}
