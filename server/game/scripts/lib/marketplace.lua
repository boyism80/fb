local M = {}

local OPT = {
    search   = '검색',
    register = '등록',
    cancel   = '취소',
}
local YES = '예'
local NO = '아니오'

function M.handle(me, npc)
::MARKETPLACE_MENU::
    local marketplace_selected, button = me:pursuit(npc, '거래소 메뉴', { OPT.search, OPT.register, OPT.cancel })
    if button == DIALOG_RESULT.QUIT then
        return true
    end

    if marketplace_selected == OPT.search then
        local result = M.search(me, npc)
        if result == false then
            return false
        end
        goto MARKETPLACE_MENU
    elseif marketplace_selected == OPT.register then
        local result = M.list(me, npc)
        if result == false then
            return false
        end
        goto MARKETPLACE_MENU
    elseif marketplace_selected == OPT.cancel then
        local result = M.cancel(me, npc)
        if result == false then
            return false
        end
        goto MARKETPLACE_MENU
    end
    return true
end

function M.search(me, npc)
::MARKETPLACE_SEARCH::
    local item_name_input = me:input(npc, '아이템 이름을 입력해주세요.')
    if item_name_input == nil or item_name_input == '' then
        return
    end

    local search_option = {}
    search_option.item_name = item_name_input
    search_option.page = 1

    local error, result = me:marketplace_search(search_option)
    if error ~= nil then
        local button = me:dialog(npc, '검색 중 오류가 발생했습니다: ' .. error, { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_SEARCH
        end
        return true
    end

    if result == nil or result.listings == nil or #result.listings == 0 then
        local button = me:dialog(npc, '검색 결과가 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_SEARCH
        end
        return true
    end

    local item_list = {}
    local listing_map = {}
    for i, listing in ipairs(result.listings) do
        local item_model = id2item(listing.item_data.model)
        if item_model ~= nil then
            local item_name = item_model:name()
            if listing_map[item_name] == nil then
                listing_map[item_name] = {}
            end
            table.insert(listing_map[item_name], listing)
            table.insert(item_list, {item_name, listing.price})
        end
    end

    if #item_list == 0 then
        local button = me:dialog(npc, '검색 결과가 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_SEARCH
        end
        return true
    end

    local selected = me:item(npc, '검색 결과입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', item_list)
    if selected == nil then
        goto MARKETPLACE_SEARCH
    end

    local selected_model = name2item(selected)
    if selected_model == nil then
        goto MARKETPLACE_SEARCH
    end

    local selected_name = selected_model:name()
    if selected_name == nil or listing_map[selected_name] == nil or #listing_map[selected_name] == 0 then
        goto MARKETPLACE_SEARCH
    end

    local selected_listing = nil
    local candidates = listing_map[selected_name]
    if #candidates == 1 then
        selected_listing = candidates[1]
    else
        local listing_list = {}
        for i, listing in ipairs(candidates) do
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
                    display_text = string.format('%s(%d개)', item_name, listing.item_data.count)
                end
            end

            display_text = string.format('%s - %d전', display_text, listing.price)
            table.insert(listing_list, display_text)
        end

        local selected_display, button = me:pursuit(npc, '같은 아이템이 여러 개 있습니다. 선택해주세요.', listing_list)
        if button == DIALOG_RESULT.QUIT then
            return false
        end

        for i, display_text in ipairs(listing_list) do
            if display_text == selected_display then
                selected_listing = candidates[i]
                break
            end
        end
    end

    if selected_listing == nil then
        goto MARKETPLACE_SEARCH
    end

    local selected_model = id2item(selected_listing.item_data.model)
    local purchase_count = 1
    if selected_model ~= nil and selected_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        local max_count = selected_listing.item_data.count
        local count_input = me:input(npc, string.format('최대 수량이 %d개입니다. 몇 개를 구매하시겠습니까?', max_count))
        if count_input == nil then
            goto MARKETPLACE_SEARCH
        end

        purchase_count = tonumber(count_input)
        if purchase_count == nil or purchase_count <= 0 or purchase_count > selected_listing.item_data.count then
            local button = me:dialog(npc, '올바른 수량을 입력해주세요.', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return false
            end
            if button == DIALOG_RESULT.PREV then
                goto MARKETPLACE_SEARCH
            end
            goto MARKETPLACE_SEARCH
        end
    end

    local warning_messages = {}
    if selected_model ~= nil then
        if selected_model:attr(ITEM_ATTRIBUTE.EQUIPMENT) and selected_listing.item_data.durability ~= nil then
            local max_durability = selected_model:durability()
            if max_durability ~= nil and max_durability > 0 then
                local durability_percent = (selected_listing.item_data.durability / max_durability) * 100
                if durability_percent < 100 then
                    table.insert(warning_messages, string.format('내구도: %d%%', math.floor(durability_percent)))
                end
            end
        end

        if selected_model:attr(ITEM_ATTRIBUTE.EQUIPMENT) and selected_model:attr(ITEM_ATTRIBUTE.WEAPON) then
            if selected_listing.item_data.custom_name ~= nil and selected_listing.item_data.custom_name ~= '' then
                table.insert(warning_messages, string.format('별칭: %s', selected_listing.item_data.custom_name))
            end
        end

        if selected_model:attr(ITEM_ATTRIBUTE.CONSUME) then
            local model_durability = selected_model:durability()
            if model_durability == nil then
                model_durability = 0
            end
            if selected_listing.item_data.durability ~= nil and selected_listing.item_data.durability ~= model_durability then
                table.insert(warning_messages, string.format('내구도: %d (기본값: %d)', selected_listing.item_data.durability, model_durability))
            end
        end
    end

    if #warning_messages > 0 then
        local warning_text = '이 아이템은 온전한 상태가 아닙니다:\n'
        for i, msg in ipairs(warning_messages) do
            warning_text = warning_text .. '- ' .. msg .. '\n'
        end
        warning_text = warning_text .. '\n정말 구매하시겠습니까?'

        local confirm_selected, confirm_button = me:pursuit(npc, warning_text, { YES, NO })
        if confirm_button == DIALOG_RESULT.QUIT then
            return false
        end

        if confirm_selected ~= YES then
            goto MARKETPLACE_SEARCH
        end
    end

    local purchase_error, purchase_result = me:marketplace_purchase(selected_listing.id, purchase_count)
    if purchase_error ~= nil then
        local button = me:dialog(npc, '구매 실패: ' .. purchase_error, { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_SEARCH
        end
        return true
    end

    local button = me:dialog(npc, '구매가 완료되었습니다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_SEARCH
    end
    return true
end

function M.list(me, npc)
::MARKETPLACE_LIST::
    local slots = {}
    local items = {}
    for slot, item in pairs(me:items()) do
        if item ~= nil then
            local model = item:model()
            if model ~= nil and model:trade() then
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
        local button = me:dialog(npc, '등록할 수 있는 아이템이 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            return true
        end
        return true
    end

    local slot_index = me:slot(npc, '무엇을 등록하시겠습니까?', slots)
    if slot_index == nil then
        return
    end

    local selected_item = items[slot_index]
    if selected_item == nil then
        goto MARKETPLACE_LIST
    end
    local selected_model = selected_item:model()
    if selected_model == nil or not selected_model:trade() then
        local button = me:dialog(npc, '등록할 수 없는 아이템입니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_LIST
        end
        goto MARKETPLACE_LIST
    end
    local count = 1

    if selected_model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        local count_input = me:input(npc, '얼마나 등록하시겠습니까?')
        if count_input == nil then
            goto MARKETPLACE_LIST
        end

        count = tonumber(count_input)
        if count == nil or count <= 0 or count > selected_item:count() then
            local button = me:dialog(npc, '올바른 수량을 입력해주세요.', { prev = true, next = true })
            if button == DIALOG_RESULT.QUIT then
                return false
            end
            if button == DIALOG_RESULT.PREV then
                goto MARKETPLACE_LIST
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
        local button = me:dialog(npc, '올바른 가격을 입력해주세요.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_LIST
        end
        goto MARKETPLACE_LIST
    end

    local total_sale_amount = count * price
    local listing_fee = math.floor(total_sale_amount * 0.05)

    local fee_message = string.format('판매금액의 5%%인 %d전이 수수료로 부과됩니다.\n등록하시겠습니까?', listing_fee)
    local confirm_selected, confirm_button = me:pursuit(npc, fee_message, { YES, NO })
    if confirm_button == DIALOG_RESULT.QUIT then
        return false
    end

    if confirm_selected ~= YES then
        goto MARKETPLACE_LIST
    end

    local list_error, list_result = me:marketplace_list(slot_index, count, price)
    if list_error ~= nil then
        local button = me:dialog(npc, '등록 실패: ' .. list_error, { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_LIST
        end
        return true
    end

    local button = me:dialog(npc, '등록이 완료되었습니다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_LIST
    end
    return true
end

function M.cancel(me, npc)
::MARKETPLACE_CANCEL::
    local my_listings_option = {}
    my_listings_option.seller_id = me:uid()
    my_listings_option.page = 1

    local error, result = me:marketplace_search(my_listings_option)
    if error ~= nil then
        local button = me:dialog(npc, '내 등록 물품 조회 중 오류가 발생했습니다: ' .. error, { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            return true
        end
        return true
    end

    if result == nil or result.listings == nil or #result.listings == 0 then
        local button = me:dialog(npc, '등록한 물품이 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            return true
        end
        return true
    end

    local item_list = {}
    local listing_map = {}
    for i, listing in ipairs(result.listings) do
        local item_model = id2item(listing.item_data.model)
        if item_model ~= nil then
            local item_name = item_model:name()
            if listing_map[item_name] == nil then
                listing_map[item_name] = {}
            end
            table.insert(listing_map[item_name], listing)
            table.insert(item_list, {item_name, listing.price})
        end
    end

    if #item_list == 0 then
        local button = me:dialog(npc, '등록한 물품이 없습니다.', { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            return true
        end
        return true
    end

    local selected = me:item(npc, '취소할 물품을 선택해주세요.', item_list)
    if selected == nil then
        goto MARKETPLACE_CANCEL
    end

    local selected_model = name2item(selected)
    if selected_model == nil then
        goto MARKETPLACE_CANCEL
    end

    local selected_name = selected_model:name()
    if selected_name == nil or listing_map[selected_name] == nil or #listing_map[selected_name] == 0 then
        goto MARKETPLACE_CANCEL
    end

    local selected_listing = nil
    local candidates = listing_map[selected_name]
    if #candidates == 1 then
        selected_listing = candidates[1]
    else
        local listing_list = {}
        for i, listing in ipairs(candidates) do
            local item_model = id2item(listing.item_data.model)
            local item_name = '알 수 없는 아이템'
            if item_model ~= nil then
                item_name = item_model:name()
            end

            local display_text = string.format('%s - %d전', item_name, listing.price)
            table.insert(listing_list, display_text)
        end

        local cancel_selected, cancel_button = me:pursuit(npc, '같은 아이템이 여러 개 있습니다. 선택해주세요.', listing_list)
        if cancel_button == DIALOG_RESULT.QUIT then
            return false
        end

        for i, display_text in ipairs(listing_list) do
            if display_text == cancel_selected then
                selected_listing = candidates[i]
                break
            end
        end
    end

    if selected_listing == nil then
        goto MARKETPLACE_CANCEL
    end

    local confirm_selected, confirm_button = me:pursuit(npc, '수수료를 반환받지 못합니다. 정말 취소하시겠습니까?', { YES, NO })
    if confirm_button == DIALOG_RESULT.QUIT then
        return false
    end

    if confirm_selected ~= YES then
        goto MARKETPLACE_CANCEL
    end

    local cancel_error, cancel_result = me:marketplace_cancel(selected_listing.id)
    if cancel_error ~= nil then
        local button = me:dialog(npc, '취소 실패: ' .. cancel_error, { prev = true, next = true })
        if button == DIALOG_RESULT.QUIT then
            return false
        end
        if button == DIALOG_RESULT.PREV then
            goto MARKETPLACE_CANCEL
        end
        goto MARKETPLACE_CANCEL
    end

    local button = me:dialog(npc, '취소가 완료되었습니다.', { prev = true, next = true })
    if button == DIALOG_RESULT.QUIT then
        return false
    end
    if button == DIALOG_RESULT.PREV then
        goto MARKETPLACE_CANCEL
    end
    goto MARKETPLACE_CANCEL
end

return M