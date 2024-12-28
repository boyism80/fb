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
    broadcast('message NOTIFY', MESSAGE_TYPE_NOTIFY)
    broadcast('message BLUE', MESSAGE_TYPE_BLUE)
    broadcast('message STATE', MESSAGE_TYPE_STATE)
    broadcast('message SHOUT', MESSAGE_TYPE_SHOUT)
    broadcast('message WORLD', MESSAGE_TYPE_WORLD)
    broadcast('message POPUP', MESSAGE_TYPE_POPUP)
    broadcast('message YELLOW', MESSAGE_TYPE_YELLOW)
    broadcast('message BROWN', MESSAGE_TYPE_BROWN)
end

function sample_group(me)
    local npc = name2npc('낙랑')
    local name = npc:input(me, '그룹 초대할 유저')
    local group = me:group()
    if group == nil then
        if me:create_group(name) then
            npc:dialog(me, '그룹이 생성되었습니다.')
        end
    else
        local found = nil
        for _, member in pairs(group:members()) do
            if member == name then
                found = member
                break
            end
        end

        if me:create_group(name) then
            if found == nil then
                npc:dialog(me, '그룹에 초대했습니다.')
            else
                npc:dialog(me, '그룹에서 추방했습니다.')
            end
        else
            npc:dialog(me, '그룹에 초대할 수 없습니다.')
        end
    end
end

function func(me)

    local npc = name2npc('낙랑')
    local clan = me:clan()
    if clan == nil then
        local name = npc:input(me, '생성할 문파 이름')
        local error = me:create_clan(name)
        if error ~= nil then
            npc:dialog(me, error)
        else
            npc:dialog(me, '클랜 생성 성공')
        end
    else
        local name = clan:name()
        npc:dialog(me, string.format('클랜 이름 : %s', name))

        local error = me:destroy_clan()
        if error ~= nil then
            npc:dialog(me, error)
        else
            npc:dialog(me, '클랜 제거 성공')
        end
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