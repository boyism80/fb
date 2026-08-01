-- spell: 강제이동(상)
-- 강제이동(상) 캐스팅

return {
    on_cast = function(me, spell)
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

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
