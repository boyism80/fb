function group_lock(me, group)
    if group == nil then
        me:chat('그룹이 없어용')
    end
end

function func(me)
    me:group(group_lock)
    
    local items = me:items()

    for slot, item in pairs(items) do
        local model = item:model()
        if model:attr(ITEM_ATTR_EQUIPMENT) and model:repair_price() ~= nil then
            local current = item:durability()
            local max = model:durability()
            item:durability(max - 1)
        end
    end

    local npc = name2npc('낙랑')
    npc:repair(me)

    local map = me:map()
    if map == nil then
        return
    end

    local nears = map:nears({me:position()})
    for _, near in pairs(nears) do
        near:chat(near:name())
    end
end