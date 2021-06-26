function handle_spell(me, spell)
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