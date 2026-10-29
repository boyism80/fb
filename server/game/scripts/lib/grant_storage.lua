local M = {}

local MAX_ITEM_SLOTS = 52

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
    if me:role() < ROLE.ADMIN then
        return true
    end

    local target_sel, btn = me:pursuit(npc, '누구에게 보낼까요?', { '전체 유저', '특정 유저' })
    if btn == DIALOG_RESULT.QUIT then
        return true
    end

    local is_global = (target_sel == 1)
    local user_name = nil
    if is_global == false then
        user_name = me:input(npc, '캐릭터 이름을 입력하세요.')
        if user_name == nil or user_name == '' then
            me:dialog(npc, '캐릭터 이름이 필요합니다.', { prev = false, next = true })
            return true
        end
    end

    local title = me:input(npc, '제목을 입력하세요.')
    if title == nil or title == '' then
        me:dialog(npc, '제목이 필요합니다.', { prev = false, next = true })
        return true
    end

    local message = me:input(npc, '내용을 입력하세요.')
    if message == nil or message == '' then
        me:dialog(npc, '내용이 필요합니다.', { prev = false, next = true })
        return true
    end

    local attachments = {
        ['item'] = {},
        ['money'] = 0,
        ['exp'] = 0,
    }

    ::REWARD_MENU::
    local reward_sel, reward_btn = me:pursuit(npc, '보상을 추가할까요?', {
        '아이템', '경험치', '금전', '없음',
    })
    if reward_btn == DIALOG_RESULT.QUIT then
        return true
    end

    if reward_sel == 1 then
        local item_name = me:input(npc, '아이템 이름을 입력하세요.')
        if item_name == nil or item_name == '' then
            me:dialog(npc, '아이템 이름이 필요합니다.', { prev = false, next = true })
            goto REWARD_MENU
        end
        local item_model = name2item(item_name)
        if item_model == nil then
            me:dialog(npc, '존재하지 않는 아이템입니다.', { prev = false, next = true })
            goto REWARD_MENU
        end

        local count_raw = me:input(npc, '수량을 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 수량입니다.', { prev = false, next = true })
            goto REWARD_MENU
        end

        local total = (attachments['item'][item_name] or 0) + count
        local limit = MAX_ITEM_SLOTS
        if item_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
            limit = item_model:capacity()
        end
        if total > limit then
            me:dialog(npc, string.format('%s 최대 %d개까지 보낼 수 있습니다.', name_with(item_name, '은', '는'), limit),
                { prev = false, next = true })
            goto REWARD_MENU
        end

        local slots = 0
        for name, value in pairs(attachments['item']) do
            if name ~= item_name then
                if name2item(name):attr(ITEM_ATTRIBUTE.BUNDLE) then
                    slots = slots + 1
                else
                    slots = slots + value
                end
            end
        end
        if item_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
            slots = slots + 1
        else
            slots = slots + total
        end
        if slots > MAX_ITEM_SLOTS then
            me:dialog(npc, string.format('한 상자에는 아이템을 %d칸까지만 넣을 수 있습니다.', MAX_ITEM_SLOTS), { prev = false, next = true })
            goto REWARD_MENU
        end

        attachments['item'][item_name] = total
        goto REWARD_MENU
    elseif reward_sel == 2 then
        local count_raw = me:input(npc, '경험치를 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 경험치입니다.', { prev = false, next = true })
            goto REWARD_MENU
        end
        attachments['exp'] = attachments['exp'] + count
        goto REWARD_MENU
    elseif reward_sel == 3 then
        local count_raw = me:input(npc, '금전을 입력하세요.')
        local count = parse_positive_number(count_raw)
        if count == nil then
            me:dialog(npc, '잘못된 금전입니다.', { prev = false, next = true })
            goto REWARD_MENU
        end
        attachments['money'] = attachments['money'] + count
        goto REWARD_MENU
    elseif reward_sel ~= 4 then
        return true
    end

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
        me:dialog(npc, '보상이 하나 이상 필요합니다.', { prev = false, next = true })
        return true
    end

    local ok = false
    local external_ref = nil
    ::SEND::
    if is_global then
        ok, external_ref = me:send_system_storage_box(title, message, payload, nil, external_ref)
    else
        ok, external_ref = me:send_storage_box(user_name, title, message, payload, nil, external_ref)
    end

    if ok then
        me:dialog(npc, '지급이 완료되었습니다.', { prev = false, next = true })
    else
        local retry_sel, retry_btn = me:pursuit(npc, '지급 결과를 확인하지 못했습니다. 같은 지급을 다시 보낼까요?', { '예', '아니오' })
        if retry_btn ~= DIALOG_RESULT.QUIT and retry_sel == 1 then
            goto SEND
        end
    end
    return true
end

return M
