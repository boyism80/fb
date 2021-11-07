function on_cast(me, spell)
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