-- spell: 강제이동(좌)
-- 강제이동(좌) 캐스팅
function ON_CAST_10002(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local x, y = me:position()
    if x == 0 then
        return
    end
    
    me:position(x-1, y)
end
