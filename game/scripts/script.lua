function func(me)
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
end