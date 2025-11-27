function ON_F1_EVENT(me)
    local npc = name2npc('낙랑')
    local entries = nil
    local current_entry = nil

    local function build_reward_text(entry)
        local reward_info = {}
        if entry.attachments then
            for _, attachment in ipairs(entry.attachments) do
                local params = attachment.Parameters
                if attachment.Type == 'item' and params and #params >= 2 then
                    table.insert(reward_info, string.format('아이템 ID:%d x%d', params[1], params[2]))
                elseif attachment.Type == 'money' and params and #params >= 1 then
                    table.insert(reward_info, string.format('금전 %d전', params[1]))
                elseif attachment.Type == 'exp' and params and #params >= 1 then
                    table.insert(reward_info, string.format('경험치 %d', params[1]))
                end
            end
        end

        if #reward_info == 0 then
            return '\n보상: 없음'
        end
        return '\n보상: ' .. table.concat(reward_info, ', ')
    end

    local function build_detail_message(entry)
        local expire_text = ''
        if entry.expired_date then
            expire_text = '\n만료일: ' .. entry.expired_date
        end

        local received_text = entry.received and '수령 완료' or '수령 가능'
        return string.format('%s%s%s\n상태: %s', entry.message, expire_text, build_reward_text(entry), received_text)
    end

::MAIN_MENU::
    local selected = me:list(npc, '무엇을 도와드릴까요?', {'통합보관함'})
    if selected == nil then
        return
    end

    if selected ~= 0 then
        return
    end

::STORAGE_LIST::
    entries = me:storage_entries()
    if entries == nil or #entries == 0 then
        if me:dialog(npc, '보관함에 보관된 항목이 없습니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MAIN_MENU
    end

    local entry_list = {}
    for _, entry in ipairs(entries) do
        table.insert(entry_list, entry.title ~= nil and entry.title ~= '' and entry.title or entry.message)
    end

    local entry_index, button = me:list(npc, '통합보관함', entry_list, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto MAIN_MENU
    end
    if entry_index == nil then
        goto STORAGE_LIST
    end

    current_entry = entries[entry_index + 1]
    if current_entry == nil then
        goto STORAGE_LIST
    end

::ENTRY_DETAIL::
    local detail_header = current_entry.title ~= nil and current_entry.title ~= '' and current_entry.title or '보관함 보상'
    local detail_button = me:dialog(npc, detail_header .. '\n' .. build_detail_message(current_entry), true, true)
    if detail_button == DIALOG_RESULT.QUIT then
        return
    end
    if detail_button == DIALOG_RESULT.PREV then
        goto STORAGE_LIST
    end
    if detail_button ~= DIALOG_RESULT.NEXT then
        goto STORAGE_LIST
    end

    if current_entry.received then
        if me:dialog(npc, '이미 수령한 보상입니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto STORAGE_LIST
    end

::RECEIVE_CONFIRM::
    local receive_selected, receive_button = me:list(npc, '보상을 수령하시겠습니까?', {'예', '아니오'}, true)
    if receive_button == DIALOG_RESULT.QUIT then
        return
    end
    if receive_button == DIALOG_RESULT.PREV then
        goto ENTRY_DETAIL
    end
    if receive_selected == nil then
        goto ENTRY_DETAIL
    end

    if receive_selected ~= 0 then
        goto STORAGE_LIST
    end

    local success = me:receive_storage_reward(current_entry.id)
    if success then
        if me:dialog(npc, '보상이 지급되었습니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
    else
        if me:dialog(npc, '수령 조건이 맞지 않습니다. 확인 후 다시 시도해주세요.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
    end
    goto STORAGE_LIST
end

function ON_F2_EVENT(me)
    local npc = name2npc('낙랑')
    local button = me:dialog(npc, 'F2 이벤트 스크립트', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end