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

function sample_message()
    broadcast('message NOTIFY', MESSAGE_TYPE.NOTIFY)
    broadcast('message BLUE', MESSAGE_TYPE.BLUE)
    broadcast('message STATE', MESSAGE_TYPE.STATE)
    broadcast('message SHOUT', MESSAGE_TYPE.SHOUT)
    broadcast('message WORLD', MESSAGE_TYPE.WORLD)
    broadcast('message POPUP', MESSAGE_TYPE.POPUP)
    broadcast('message YELLOW', MESSAGE_TYPE.YELLOW)
    broadcast('message BROWN', MESSAGE_TYPE.BROWN)
end

function func(me, val1, val2, val3)
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
    -- sleep(1000)

    me:mimic(nil)

    return 'hello', 'world'
end