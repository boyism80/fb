-- item: 해독의귀걸이

return {
    -- ON_ACTIVATED = function(me, item)
    -- end,

    -- ON_DEACTIVATED = function(me, item)
    -- end,

    ON_CONCAST = function(me, item)
        if me:isbuff('중독') then
            me:unbuff('중독')
        end
    end,

    -- ON_ATTACK = function(me, item)
    -- end
}
