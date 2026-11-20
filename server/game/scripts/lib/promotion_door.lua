local quest = require('lib.quest')

local M = {}

local function sight_group(me)
    local group = me:group()
    local map = me:map()
    if group == nil or map == nil then
        return {}
    end
    local found = {}
    for _, ch in pairs(group:nears(map, { me:position() })) do
        if ch ~= me then
            found[#found + 1] = ch
        end
    end
    return found
end

function M.warp_group(me, map, x, y)
    local entered = false
    for _, ch in ipairs(sight_group(me)) do
        if ch:map(map, x, y) == true then
            entered = true
        end
    end
    if me:map(map, x, y) == true then
        entered = true
    end
    return entered
end

function M.key_door(me, item_name, lack_message, map_id, x, y)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    if not me:has_items(item_name, 1) then
        me:message(lack_message, MESSAGE_TYPE.STATE)
        return
    end
    local map = id2map(map_id)
    if map == nil then
        return
    end
    if not me:rmitem(item_name, 1, ITEM_DELETE_TYPE.GIVE) then
        return
    end
    if not M.warp_group(me, map, x, y) then
        me:mkitem(item_name, 1)
    end
end

local function near_named(me, name)
    for _, ch in pairs(me:nears(OBJECT_TYPE.CHARACTER)) do
        if ch:name() == name then
            return ch
        end
    end
    return nil
end

local function land_of(spec)
    local x = spec.x1
    local y = spec.y1
    if spec.x2 ~= nil and spec.x2 ~= spec.x1 then
        x = math.random(spec.x1, spec.x2)
    end
    if spec.y2 ~= nil and spec.y2 ~= spec.y1 then
        y = math.random(spec.y1, spec.y2)
    end
    return x, y
end

function M.boss_door(me, spec)
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return
    end
    local q = me:quest(quest.QUEST_MUTA)
    local step = nil
    if q ~= nil and not q:completed() then
        step = q:step()
    end
    local allowed = false
    if step ~= nil then
        if spec.exact ~= nil then
            allowed = step == spec.exact
        else
            allowed = step >= spec.minimum
        end
    end
    if not allowed then
        me:message('들어갈 수 없습니다.', MESSAGE_TYPE.STATE)
        return
    end
    local map = id2map(spec.map)
    if map == nil then
        return
    end

    local selected, button = me:list(me, '함께 온 동료가 있으면 같이 들여보내 주겠다.\n동료가 있느냐?', { '네. 있습니다.', '아니오. 없습니다.' })
    if button == DIALOG_RESULT.QUIT or selected == nil then
        return
    end

    local party = {}
    if selected == 1 then
        while true do
            local name = me:input(me, '동료의 이름은?', { top = '동료의 이름은', bottom = '입니다.', maxlen = 18, prev = false })
            if name == DIALOG_RESULT.QUIT or name == nil or name == '' then
                return
            end
            local ch = name2ch(name, me)
            local duplicated = false
            for _, old in ipairs(party) do
                if old:name() == name then
                    duplicated = true
                end
            end
            if ch == nil or ch:name() == me:name() or duplicated or me:map() ~= ch:map() or near_named(me, ch:name()) == nil then
                local denied = me:dialog(nil, '그 동료는 이곳에 없다.', { prev = false, next = true })
                if denied == DIALOG_RESULT.QUIT then
                    return
                end
            elseif ch:script('scripts/warp/promotion.lua', 'on_confirm', me:name()) ~= true then
                local denied = me:dialog(nil, '동료가 따라오지 않는다.', { prev = false, next = true })
                if denied == DIALOG_RESULT.QUIT then
                    return
                end
            else
                party[#party + 1] = ch
            end

            local more, more_button = me:list(me, '또다른 동료가 있느냐?', { '네. 있습니다.', '아니오. 없습니다.' })
            if more_button == DIALOG_RESULT.QUIT or more == nil then
                return
            end
            if more ~= 1 then
                break
            end
        end
    end

    for _, ch in ipairs(party) do
        if me:map() ~= ch:map() or near_named(me, ch:name()) == nil then
            me:dialog(nil, '근처에 없는 동료가 있다.', { prev = false, next = false })
            return
        end
    end

    local x, y = land_of(spec)
    for _, ch in ipairs(party) do
        ch:map(map, x, y)
    end
    me:map(map, x, y)
end

function M.oak_skip(me, npc, map_id, x, y)
    if not me:has_items('참나무조각', 1) then
        me:dialog(npc, '참나무조각을 가지고 오면 보스방으로 들여보내 주겠다.', { prev = false, next = false })
        return
    end
    local selected, button = me:list(npc, '참나무조각을 바치면 보스방으로 들여보내 주겠다. 시야 안의 동료도 함께 들여보낸다.', {
        '바치겠습니다.',
        '나중에 오겠습니다.',
    })
    if button == DIALOG_RESULT.QUIT or selected ~= 1 then
        return
    end
    local map = id2map(map_id)
    if map == nil then
        return
    end
    if not me:rmitem('참나무조각', 1, ITEM_DELETE_TYPE.GIVE) then
        return
    end
    if not M.warp_group(me, map, x, y) then
        me:mkitem('참나무조각', 1)
    end
end

return M
