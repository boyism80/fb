function buy(me, npc, pursuit)
    local purchase_list = {}
    if type(pursuit) == 'number' then
        for i, pair in pairs(pursuit_buy(pursuit)) do
            local item, price = table.unpack(pair)
            purchase_list[item:name()] = price
        end
    else
        return npc:dialog(me, '알 수 없는 에러')
    end

    local slots = {}
    local items = {}
    local my_items = me:items()
    for slot, item in pairs(my_items) do
        local model = item:model()
        local name = model:name()
        if purchase_list[model:name()] ~= nil then
            table.insert(slots, slot)
            items[slot] = model
        end
    end

::ROUTINE0000::
    local slot = npc:slot(me, '뭘 팔래요?', slots)
    if slot == nil then
        return DIALOG_RESULT_NEXT
    end

    local item = items[slot]
    local price = purchase_list[item:name()]
    local count = 1
    if item:attr(ITEM_ATTR_BUNDLE) then
        count = npc:input(me, '몇개나 파시겠어요?')
        if count == nil then
            goto ROUTINE0000
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return npc:dialog(me, '갯수가 올바르지 않습니다.', false, true)
        end
    end
    price = price * count

    local message = nil
    if count > 1 then
        message = string.format('%s %d개를 %d에 파시겠습니까?', item:name(), count, price)
    else
        message = string.format('%s %d에 파시겠습니까?', name_with(item:name()), price)
    end
    if npc:menu(me, message, {'네', '아니오'}) == 0 then
        me:money(me:money() + price)
        me:rmitem(slot, count, ITEM_DELETE_ATTR_SELL)
    end
    return DIALOG_RESULT_NEXT
end


function sell(me, npc, pursuit)
    if pursuit == nil then
        return DIALOG_RESULT_QUIT
    end

    if type(pursuit) == 'table' then
        local menu = {}
        for i, pair in pairs(pursuit) do
            local m, _ = table.unpack(pair)
            table.insert(menu, m)
        end
::ROUTINE0001::
        local selected = npc:menu(me, '무엇을 사시겠어요?', menu)
        if selected == nil then
            goto ROUTINE0001
        end

        local _, p = table.unpack(pursuit[selected+1])
        local action = sell(me, npc, p)
        if action == DIALOG_RESULT_NEXT then
            goto ROUTINE0001
        end

        return action
    end

    local list = pursuit_sell(pursuit)
::ROUTINE0002::
    local selected = npc:item(me, '제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', list)
    if selected == nil then
        return DIALOG_RESULT_NEXT
    end

    local count = 1
    local item = name2item(selected)
    local is_bundle = item:attr(ITEM_ATTR_BUNDLE)
    if is_bundle then
        count = npc:input(me, '몇개나 사시겠어요?')
        if count == nil then
            goto ROUTINE0002
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return npc:dialog(me, '갯수가 올바르지 않습니다.', false, true)
        end
    end

    local price = sell_price(pursuit, selected)
    if price == nil then
        return npc:dialog(me, '알 수 없는 에러')
    else
        price = price * count
    end

    local money = me:money()
    if price > money then
        return npc:dialog(me, '돈이 모자랍니다.', false, true)
    end

    local exist_count = 0
    local exist = me:item(selected)
    if exist ~= nil then
        exist_count = exist:count()
    end

    if is_bundle and exist_count + count > item:capacity() then
        return npc:dialog(me, '더 이상 가질 수 없습니다.', false, true)
    end
    
    if me:mkitem(selected, count) == nil then
        return npc:dialog(me, '공간이 부족합니다.', false, true)
    else
        me:money(money - price)
        local message = nil
        if count > 1 then
            message = string.format('%s %d개를 %d전에 팔았습니다.', selected, count, price)
        else
            message = string.format('%s %d전에 팔았습니다.', name_with(selected), price)
        end
        return npc:dialog(me, message, false, true)
    end
end



function repairable_slots(me)
    local items = me:items()
    local slots = {}
    local count = 0

    for slot, item in pairs(items) do
        local model = item:model()
        if model:attr(ITEM_ATTR_EQUIPMENT) and model:repair_price() ~= nil then
            local current = item:durability()
            local max = model:durability()
            if current < max then
                slots[slot] = item
                count = count + 1
            end
        end
    end

    if count == 0 then
        return nil
    end

    return slots
end



function repair(me, npc)
    local items = repairable_slots(me)
    if items == nil then
        return npc:dialog(me, '고칠 물건이 없는데요', false, true)
    end

    local slots = {}
    for slot, item in pairs(items) do
        table.insert(slots, slot)
    end

    local selected = npc:slot(me, '무엇을 고치시겠습니까?', slots)
    if selected == nil then
        return DIALOG_RESULT_NEXT
    end

    local item = items[selected]
    local model = item:model()
    local price = math.floor(model:repair_price() * (model:durability() - item:durability()))

    if price < 10 then
        item:durability(model:durability())
        return npc:dialog(me, '거의 새거라 그냥 고쳐드렸습니다. 잘 쓰세요')
    else
        if npc:menu(me, string.format('고치는데 %d전이 필요합니다. 정말 수리하시겠습니까?', price), {'네', '아니오'}) == 0 then
            local money = me:money()
            if price > money then
                return npc:dialog(me, '돈이 모자랍니다.', false, true)
            else
                me:money(money - price)
                item:durability(model:durability())
                return DIALOG_RESULT_NEXT
            end
        else
            return DIALOG_RESULT_NEXT
        end
    end
end



function repair_all(me, npc)
    local items = repairable_slots(me)
    if items == nil then
        return npc:dialog(me, '고칠 물건이 없는데요', false, true)
    end

    local price = 0
    for slot, item in pairs(items) do
        local model = item:model()
        local current = item:durability()
        local max = model:durability()
        price = price + (model:repair_price() * (max - current))
    end
    price = math.floor(price)

    if price < 10 then
        for slot, item in pairs(items) do
            local model = item:model()
            item:durability(model:durability())
        end
        return npc:dialog(me, '거의 새거라 그냥 고쳐드렸습니다. 잘 쓰세요')
    else
        local selected = npc:menu(me, string.format('모두 고치는데 %d전이 필요합니다. 고치시겠습니까?', price), {'네', '아니오'})
        if selected == 0 then
            local money = me:money()
            if money < price then
                return npc:dialog(me, '돈이 모자랍니다.')
            else
                for slot, item in pairs(items) do
                    local model = item:model()
                    item:durability(model:durability())
                end

                me:money(money - price)
                return npc:dialog(me, '모두 고쳤습니다.')
            end
        else
            return DIALOG_RESULT_NEXT
        end
    end
end