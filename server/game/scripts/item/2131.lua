-- item: 시력회복의귀걸이

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    ON_CONCAST = function(me, item)
        if me:isbuff('절망') then
            me:unbuff('절망')
        end
    end,

    -- ON_ATTACK = function(me, item)
    -- end
}
