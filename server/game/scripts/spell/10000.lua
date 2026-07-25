-- spell: 강제이동(상)
-- 강제이동(상) 캐스팅

return {
    ON_CAST = function(me, spell)
        local map = me:map()
        if map == nil then
            return
        end

        local x, y = me:position()
        if y == 0 then
            return
        end

        me:position(x, y-1)
    end,

    -- ON_BUFF = function(me, sp)
    -- end,

    -- ON_UNBUFF = function(me, sp)
    -- end,

    -- ON_CONCAST = function(me, sp)
    -- end
}
