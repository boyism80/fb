function ON_F1_EVENT(me)
    local npc = name2npc('낙랑')
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
        
        -- 보상이 있는 경우에만 보상과 상태 표시
        if has_attachments(entry) then
            reward_text = build_reward_text(entry)
            local received_text = entry.received and '수령 완료' or '수령 가능'
            status_text = '\n\n상태: ' .. received_text
        end
        
        return string.format('%s%s%s%s', entry.message, expire_text, reward_text, status_text)
    end

::MAIN_MENU::
    local selected = me:list(npc, '무엇을 도와드릴까요?', {'통합보관함', '거래소'})
    if selected == nil then
        return
    end

    if selected == 0 then
        goto STORAGE_LIST
    elseif selected == 1 then
        goto MARKETPLACE_MENU
    else
        return
    end

::MARKETPLACE_MENU::
    local marketplace_selected = me:list(npc, '거래소 메뉴', {'검색', '등록', '취소'})
    if marketplace_selected == nil then
        return
    end

    if marketplace_selected == 0 then
        goto MARKETPLACE_SEARCH
    elseif marketplace_selected == 1 then
        goto MARKETPLACE_LIST
    elseif marketplace_selected == 2 then
        goto MARKETPLACE_CANCEL
    else
        goto MAIN_MENU
    end

::MARKETPLACE_SEARCH::
    local item_id_input = me:input(npc, '아이템 ID를 입력해주세요.')
    if item_id_input == nil then
        goto MARKETPLACE_MENU
    end

    local item_id = tonumber(item_id_input)
    if item_id == nil or item_id <= 0 then
        if me:dialog(npc, '올바른 아이템 ID를 입력해주세요.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_SEARCH
    end

    local model = id2item(item_id)
    if model == nil then
        if me:dialog(npc, '존재하지 않는 아이템 ID입니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_SEARCH
    end

    local search_option = {}
    search_option.item_name = model:name()
    search_option.page = 1

    local error, result = me:marketplace_search(search_option)
    if error ~= nil then
        if me:dialog(npc, '검색 중 오류가 발생했습니다: ' .. error, true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    if result == nil or result.listings == nil or #result.listings == 0 then
        if me:dialog(npc, '검색 결과가 없습니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    local item_list = {}
    for i, listing in ipairs(result.listings) do
        local item_model = id2item(listing.item_data.model)
        local item_name = '알 수 없는 아이템'
        if item_model ~= nil then
            item_name = item_model:name()
        end

        local display_text = item_name
        if item_model ~= nil then
            if item_model:attr(ITEM_ATTRIBUTE.EQUIPMENT) and listing.item_data.durability ~= nil then
                local max_durability = item_model:durability()
                if max_durability ~= nil and max_durability > 0 then
                    local durability_percent = math.floor((listing.item_data.durability / max_durability) * 100)
                    display_text = string.format('%s(%d%%)', item_name, durability_percent)
                end
            elseif item_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
                local max_count = item_model:capacity()
                if max_count ~= nil then
                    display_text = string.format('%s(최대 %d개)', item_name, max_count)
                end
            end
        end

        display_text = string.format('%s - %d전', display_text, listing.price)
        table.insert(item_list, display_text)
    end

    local selected_index, button = me:list(npc, '검색 결과', item_list, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end
    if button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_MENU
    end
    if selected_index == nil then
        goto MARKETPLACE_SEARCH
    end

    local selected_listing = result.listings[selected_index + 1]
    if selected_listing == nil then
        goto MARKETPLACE_SEARCH
    end

    local selected_model = id2item(selected_listing.item_data.model)
    local purchase_count = 1
    if selected_model ~= nil and selected_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        local max_count = selected_model:capacity()
        local count_input = me:input(npc, string.format('최대 수량이 %d개입니다. 몇 개를 구매하시겠습니까?', max_count))
        if count_input == nil then
            goto MARKETPLACE_SEARCH
        end

        purchase_count = tonumber(count_input)
        if purchase_count == nil or purchase_count <= 0 or purchase_count > max_count then
            if me:dialog(npc, '올바른 수량을 입력해주세요.', true, false) == DIALOG_RESULT.QUIT then
                return
            end
            goto MARKETPLACE_SEARCH
        end
    end

    local purchase_error, purchase_result = me:marketplace_purchase(selected_listing.id)
    if purchase_error ~= nil then
        if me:dialog(npc, '구매 실패: ' .. purchase_error, true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    if me:dialog(npc, '구매가 완료되었습니다.', true, false) == DIALOG_RESULT.QUIT then
        return
    end
    goto MARKETPLACE_MENU

::MARKETPLACE_LIST::
    local slots = {}
    local items = {}
    for slot, item in pairs(me:items()) do
        if item ~= nil then
            local model = item:model()
            if model ~= nil then
                local item_name = model:name()
                if item:count() > 1 then
                    item_name = string.format('%s x%d', item_name, item:count())
                end
                table.insert(slots, slot)
                items[slot] = item
            end
        end
    end

    if #slots == 0 then
        if me:dialog(npc, '등록할 수 있는 아이템이 없습니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    local slot_index = me:slot(npc, '무엇을 등록하시겠습니까?', slots)
    if slot_index == nil then
        goto MARKETPLACE_MENU
    end

    local selected_item = items[slot_index]
    if selected_item == nil then
        goto MARKETPLACE_LIST
    end
    local selected_model = selected_item:model()
    local count = 1

    if selected_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        local count_input = me:input(npc, '얼마나 등록하시겠습니까?')
        if count_input == nil then
            goto MARKETPLACE_LIST
        end

        count = tonumber(count_input)
        if count == nil or count <= 0 or count > selected_item:count() then
            if me:dialog(npc, '올바른 수량을 입력해주세요.', true, false) == DIALOG_RESULT.QUIT then
                return
            end
            goto MARKETPLACE_LIST
        end
    end

    local price_input = me:input(npc, '가격을 입력해주세요.')
    if price_input == nil then
        goto MARKETPLACE_LIST
    end

    local price = tonumber(price_input)
    if price == nil or price <= 0 then
        if me:dialog(npc, '올바른 가격을 입력해주세요.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_LIST
    end

    local list_error, list_result = me:marketplace_list(slot_index, count, price)
    if list_error ~= nil then
        if me:dialog(npc, '등록 실패: ' .. list_error, true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    if me:dialog(npc, '등록이 완료되었습니다.', true, false) == DIALOG_RESULT.QUIT then
        return
    end
    goto MARKETPLACE_MENU

::MARKETPLACE_CANCEL::
    local my_listings_option = {}
    my_listings_option.seller_id = me:id()
    my_listings_option.page = 1

    local error, result = me:marketplace_search(my_listings_option)
    if error ~= nil then
        if me:dialog(npc, '내 등록 물품 조회 중 오류가 발생했습니다: ' .. error, true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    if result == nil or result.listings == nil or #result.listings == 0 then
        if me:dialog(npc, '등록한 물품이 없습니다.', true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_MENU
    end

    local listing_list = {}
    for i, listing in ipairs(result.listings) do
        local item_model = id2item(listing.item_data.model)
        local item_name = '알 수 없는 아이템'
        if item_model ~= nil then
            item_name = item_model:name()
        end

        local display_text = string.format('%s - %d전', item_name, listing.price)
        table.insert(listing_list, display_text)
    end

    local cancel_selected_index, cancel_button = me:list(npc, '취소할 물품을 선택해주세요.', listing_list, true)
    if cancel_button == DIALOG_RESULT.QUIT then
        return
    end
    if cancel_button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_MENU
    end
    if cancel_selected_index == nil then
        goto MARKETPLACE_CANCEL
    end

    local selected_listing = result.listings[cancel_selected_index + 1]
    if selected_listing == nil then
        goto MARKETPLACE_CANCEL
    end

    local confirm_selected, confirm_button = me:list(npc, '수수료를 반환받지 못합니다. 정말 취소하시겠습니까?', {'예', '아니오'}, true)
    if confirm_button == DIALOG_RESULT.QUIT then
        return
    end
    if confirm_button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_CANCEL
    end
    if confirm_selected == nil then
        goto MARKETPLACE_CANCEL
    end

    if confirm_selected ~= 0 then
        goto MARKETPLACE_CANCEL
    end

    local cancel_error, cancel_result = me:marketplace_cancel(selected_listing.id)
    if cancel_error ~= nil then
        if me:dialog(npc, '취소 실패: ' .. cancel_error, true, false) == DIALOG_RESULT.QUIT then
            return
        end
        goto MARKETPLACE_CANCEL
    end

    if me:dialog(npc, '취소가 완료되었습니다.', true, false) == DIALOG_RESULT.QUIT then
        return
    end
    goto MARKETPLACE_CANCEL

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
    local detail_message = build_detail_message(current_entry)
    
    -- 보상이 없는 경우 상세 정보를 보여주고 바로 리스트로 돌아감
    if not has_attachments(current_entry) then
        local detail_button = me:dialog(npc, detail_header .. '\n' .. detail_message, true, true)
        if detail_button == DIALOG_RESULT.QUIT then
            return
        end
        goto STORAGE_LIST
    end

    local detail_button = me:dialog(npc, detail_header .. '\n' .. detail_message, true, true)
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
        if me:dialog(npc, '보상이 지급되었습니다.', true, true) == DIALOG_RESULT.QUIT then
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