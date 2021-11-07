function on_cast(me, spell)
    local map = me:map()
    if map == nil then
        return
    end

    local x, y = me:position()
    if y == map:height() - 1 then
        return
    end
    
    me:position(x, y+1)
end