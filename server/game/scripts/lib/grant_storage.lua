local M = {}

local function parse_positive_number(raw)
    if raw == nil or raw == '' then
        return nil
    end
    local n = tonumber(raw)
    if n == nil or n <= 0 or n ~= math.floor(n) then
        return nil
    end
    return n
end

function M.handle(me, npc)
    local target_sel, btn = me:list(npc, '누구에게 보낼까요?', {'전체 유저', '특정 유저'}, true)
    if btn == DIALOG_RESULT.QUIT then
        return false
    end
    if btn == DIALOG_RESULT.PREV then
        return true
    end
    if target_sel == nil then
        return true
    end

    local is_global = (target_sel == 0)
    local user_name = nil
    if is_global == false then
        user_name = me:input(npc, '캐릭터 이름을 입력하세요.')
        if user_name == nil or user_name == '' then
            me:dialog(npc, '캐릭터 이름이 필요합니다.', false, true)
            return true
        end
    end

    local title = me:input(npc, '제목을 입력하세요.')
    if title == nil or title == '' then
        me:dialog(npc, '제목이 필요합니다.', false, true)
        return true
    end

    local message = me:input(npc, '내용을 입력하세요.')
    if message == nil or message == '' then
        me:dialog(npc, '내용이 필요합니다.', false, true)
        return true
    end

    local attachments = {
        ['item'] = {},
        ['money'] = 0,
        ['exp'] = 0,
    }

::REWARD_MENU::
    local reward_sel, reward_btn = me:list(npc, '보상을 추가할까요?', {'아이템', '경험치', '금전', '없음'}, false)
    if reward_btn == DIALOG_RESULT.QUIT then
        return false
    end
    if reward_sel == nil then
        return true
    end

    if reward_sel == 0 then
        local item_name = me:input(npc, '아이템 이름을 입력하세요.')
        if item_name == nil or item_name == '' then
            me:dialog(npc, '아이템 이름이 필요합니다.', false, true)
            goto REWARD_MENU
        end
        if name2item(item_name) == nil then
            me:dialog(npc, '존재하지 않는 아이템입니다.', false, true)
            goto REWARD_MENU
        end

        local count_raw = me:input(npc, '수량을 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 수량입니다.', false, true)
            goto REWARD_MENU
        end

        attachments['item'][item_name] = (attachments['item'][item_name] or 0) + count
        goto REWARD_MENU
    elseif reward_sel == 1 then
        local count_raw = me:input(npc, '경험치를 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 경험치입니다.', false, true)
            goto REWARD_MENU
        end
        attachments['exp'] = attachments['exp'] + count
        goto REWARD_MENU
    elseif reward_sel == 2 then
        local count_raw = me:input(npc, '금전을 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 금전입니다.', false, true)
            goto REWARD_MENU
        end
        attachments['money'] = attachments['money'] + count
        goto REWARD_MENU
    end

    -- reward_sel == 3 (없음): proceed to grant
    local payload = {}
    local has_item = false
    for _, _ in pairs(attachments['item']) do
        has_item = true
        break
    end
    if has_item then
        payload['item'] = attachments['item']
    end
    if attachments['money'] > 0 then
        payload['money'] = attachments['money']
    end
    if attachments['exp'] > 0 then
        payload['exp'] = attachments['exp']
    end

    if payload['item'] == nil and payload['money'] == nil and payload['exp'] == nil then
        me:dialog(npc, '보상이 하나 이상 필요합니다.', false, true)
        return true
    end

    local ok = false
    if is_global then
        ok = me:send_system_storage_box(title, message, payload)
    else
        ok = me:send_storage_box(user_name, title, message, payload)
    end

    if ok then
        me:dialog(npc, '지급이 완료되었습니다.', false, true)
    else
        me:dialog(npc, '지급에 실패했습니다.', false, true)
    end
    return true
end

return M
