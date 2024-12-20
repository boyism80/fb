function group_lock(me, group)
    if group == nil then
        me:chat('그룹이 없어용')
        return false
    end

    local map = me:map()
    if map == nil then
        return false
    end

    local nears = group:nears(me:map(), {me:position()})
    for _, near in pairs(nears) do
        near:chat(near:name())
    end
    return true, #nears
end

function func(me)
    for i=0,100 do
        me:push_trace(i)
    end

    -- local success, size = me:group(group_lock)
    -- local npc = name2npc('낙랑')
    -- npc:dialog(me, string.format('%s, %s', success, size))
    
    -- local items = me:items()

    -- for slot, item in pairs(items) do
    --     local model = item:model()
    --     if model:attr(ITEM_ATTR_EQUIPMENT) and model:repair_price() ~= nil then
    --         local current = item:durability()
    --         local max = model:durability()
    --         item:durability(max - 1)
    --     end
    -- end

    -- npc:repair(me)

    -- local map = me:map()
    -- if map == nil then
    --     return
    -- end

    -- local nears = map:nears({me:position()})
    -- for _, near in pairs(nears) do
    --     near:chat(near:name())
    -- end
end