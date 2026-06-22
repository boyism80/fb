-- spell: 강제이동(우)
-- 강제이동(우) 캐스팅
function ON_CAST_10003(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local x, y = me:position()
    if x == map:width() - 1 then
        return
    end
    
    me:position(x+1, y)
end
