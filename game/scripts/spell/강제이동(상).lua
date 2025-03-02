-- 강제이동(상) 캐스팅
function ON_CAST_10000(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local x, y = me:position()
    if y == 0 then
        return
    end
    
    me:position(x, y-1)
end