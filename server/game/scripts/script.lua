return {
    group_lock = function(me, group)
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
    end,

    sample_message = function()
        broadcast('message NOTIFY', MESSAGE_TYPE.NOTIFY)
        broadcast('message BLUE', MESSAGE_TYPE.BLUE)
        broadcast('message STATE', MESSAGE_TYPE.STATE)
        broadcast('message SHOUT', MESSAGE_TYPE.SHOUT)
        broadcast('message WORLD', MESSAGE_TYPE.WORLD)
        broadcast('message POPUP', MESSAGE_TYPE.POPUP)
        broadcast('message YELLOW', MESSAGE_TYPE.YELLOW)
        broadcast('message BROWN', MESSAGE_TYPE.BROWN)
    end,

    func = function(me, val1, val2, val3)
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

        -- me:mimic(nil)
        -- return 'hello', 'world'

        local x, y = me:position()
        local boss = me:spawn_mob('고래', x, y, false)
        boss:parts_mode(MOB_PARTS_MODE.BODY)
        boss:hide(true)

        for i = 1, 19 do
            local part = me:spawn_mob('고래' .. i, i - 1, 10, false)
            boss:parts(part)
        end

        -- local x, y = me:position()
        -- local boss = me:spawn_mob('다람쥐', x, y, false)
        -- boss:parts_mode(MOB_PARTS_MODE.PARTS)

        -- local part1 = me:spawn_mob('토끼', x, y + 1, false)
        -- local part2 = me:spawn_mob('토끼', x, y + 2, false)
        -- boss:parts(part1)
        -- boss:parts(part2)
    end
}
