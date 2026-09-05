local M = {}

function M.handle(me, npc)
    local entries = nil
    local current_entry = nil
    
    local function has_attachments(entry)
        return entry.attachments ~= nil and #entry.attachments > 0
    end
    
    local function build_reward_text(entry)
        local lines = {}
        if has_attachments(entry) then
            for _, attachment in ipairs(entry.attachments) do
                local params = attachment.Params
                if attachment.Header == 'item' and params and #params >= 2 then
                    local id = params[1]
                    local count = params[2]
                    local model = id2item(id)
                    local name = nil
                    if model ~= nil then
                        name = model:name()
                    else
                        name = string.format('아이템 ID:%d', id)
                    end
                    
                    if count ~= nil and count >= 2 then
                        table.insert(lines, string.format(' - %s x%d', name, count))
                    else
                        table.insert(lines, string.format(' - %s', name))
                    end
                elseif attachment.Header == 'money' and params and #params >= 1 then
                    table.insert(lines, string.format(' - 금전 %d전', params[1]))
                elseif attachment.Header == 'exp' and params and #params >= 1 then
                    table.insert(lines, string.format(' - 경험치 %d', params[1]))
                end
            end
        end
        
        if #lines == 0 then
            return '\n\n보상: 없음'
        end
        
        return '\n\n보상:\n' .. table.concat(lines, '\n')
    end
    
    local function build_detail_message(entry)
        local expire_text = ''
        if entry.expired_date then
            expire_text = '\n만료일: ' .. entry.expired_date
        end
        
        local reward_text = ''
        local status_text = ''
        
        if has_attachments(entry) then
            reward_text = build_reward_text(entry)
            local received_text = entry.received and '수령 완료' or '수령 가능'
            status_text = '\n\n상태: ' .. received_text
        end
        
        return string.format('%s%s%s%s', entry.message, expire_text, reward_text, status_text)
    end
    
::STORAGE_LIST::
    entries = me:storage_entries()
    if entries == nil or #entries == 0 then
        local button = me:dialog(npc, '보관함에 보관된 항목이 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            return true
        end
        return true
    end
    
    local entry_list = {}
    for _, entry in ipairs(entries) do
        table.insert(entry_list, entry.title ~= nil and entry.title ~= '' and entry.title or entry.message)
    end
    
    local entry_index, button = me:pursuit(npc, '통합보관함', entry_list)
    if button == DIALOG_RESULT.QUIT then
        return true
    end

    current_entry = entries[entry_index]
    if current_entry == nil then
        goto STORAGE_LIST
    end
    
::ENTRY_DETAIL::
    local detail_header = current_entry.title ~= nil and current_entry.title ~= '' and current_entry.title or '보관함 보상'
    local detail_message = build_detail_message(current_entry)

    if not has_attachments(current_entry) then
        local detail_button = me:dialog(npc, detail_header .. '\n' .. detail_message, { prev = true, next = true })
        if detail_button == DIALOG_RESULT.QUIT then
            return false
        end
        if detail_button == DIALOG_RESULT.PREV then
            goto STORAGE_LIST
        end
        goto STORAGE_LIST
    end

    local detail_button = me:dialog(npc, detail_header .. '\n' .. detail_message, { prev = true, next = true })
    if detail_button == DIALOG_RESULT.QUIT then
        return false
    end
    if detail_button == DIALOG_RESULT.PREV then
        goto STORAGE_LIST
    end
    if detail_button ~= DIALOG_RESULT.NEXT then
        goto STORAGE_LIST
    end

    if current_entry.received then
        goto STORAGE_LIST
    end

::RECEIVE_CONFIRM::
    local receive_selected, receive_button = me:pursuit(npc, '보상을 수령하시겠습니까?', { '예', '아니오' })
    if receive_button == DIALOG_RESULT.QUIT then
        return true
    end
    if receive_selected ~= 1 then
        goto STORAGE_LIST
    end

    local success = me:receive_storage_reward(current_entry.id)
    if success then
        local button = me:dialog(npc, '보상이 지급되었습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto STORAGE_LIST
        end
    else
        local button = me:dialog(npc, '수령 조건이 맞지 않습니다. 확인 후 다시 시도해주세요.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto STORAGE_LIST
        end
    end
    goto STORAGE_LIST
end

return M
