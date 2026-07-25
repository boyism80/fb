-- spell: 강제이동(좌)
-- 강제이동(좌) 캐스팅

return {
    on_cast = function(me, spell)
        local map = me:map()
        if map == nil then
            return
        end

        local x, y = me:position()
        if x == 0 then
            return
        end

        me:position(x-1, y)
    end,

    -- on_buff = function(me, sp)
    -- end,

    -- on_unbuff = function(me, sp)
    -- end,

    -- on_concast = function(me, sp)
    -- end
}
