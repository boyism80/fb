function npc_revive(me, npc, discourteous)
    if me:state() ~= STATE_GHOST then
        return true
    end

    if discourteous then
        npc:chat('곱게 빌어도 살려줄까 말까 하는데...')
        return false
    end

    local i = math.random(0, 5)
    if i == 0 then
        npc:chat('생명의 소중함을 그렇게 가르쳤거늘...')
    elseif i == 1 then
        npc:chat('생즉시공 공즉시생')
    elseif i == 2 then
        npc:chat('인생의 뜻을 모르고 있구나..')
    elseif i == 3 then
        npc:chat('정성이 부족하구나...')
    elseif i == 4 then
        npc:chat('그런 마음가짐으로 생명을 얻겠느냐?')
    else
        npc:chat('너의 정성에 감복하여 살려줄터이니 잠시 기다려라....')
        sleep(1000)
        npc:chat('영은 생을 얻을지어니...')
        me:state(STATE_NORMAL)
        me:hp(50)
    end
    return true
end

function npc_appreciate(me, npc)
    if me:hp() > 100 then
        me:chat(me:hp())
        return true
    end

    npc:chat('예의 바른 놈. 잘 살거라.')
    me:hp(me:maxhp())
    return true
end

function npc_hold_item_count(me, npc, name)
    local model = name2item(name)
    if model == nil then
        return true
    end

    local item = me:deposited_item(model)
    if item == nil then
        npc:chat('그런 물건은 맡고 있지 않습니다.') -- MESSAGE_NO_ITEM_DEPOSITED
        return true
    end

    npc:chat(string.format('%s %d개 맡고 있습니다.', name_with(name), item:count()))
    return true
end

function npc_hold_item_list(me, npc)
    local items = me:deposited_item()
    local count = #items
    if count == 0 then
        npc:chat('맡긴 물건이 없습니다.') -- MESSAGE_NO_ANY_DEPOSITED
        return true
    end

    local overflow = count
    local names = {}
    for i = 1, math.min(3, count) do
        table.insert(names, items[i]:model():name())
        overflow = overflow - 1
    end

    local message = table.concat(names, ", ")
    if overflow > 0 then
        message = string.format('%s 외 %d개를 맡고 있습니다.', message, overflow)
    else
        message = string.format('%s 맡고 있습니다.', name_with(message))
    end
    npc:chat(message)
    return true
end

function npc_rename_weapon(me, npc, from, to)
    local model = name2item(from)
    if model == nil then
        return true
    end

    if model:attr(ITEM_ATTRIBUTE_WEAPON) == false then
        npc:chat(string.format('%s 무기가 아닙니다.', name_with(model:name(), '은', '는')))
        return true
    end

    local weapon = me:item(from)
    if weapon == nil then
        npc:chat(string.format('%s 가지고 있지 않습니다.', name_with(model:name(), '을', '를')))
        return true
    end

    local rename_price = model:rename_price()
    if rename_price == nil then
        npc:chat(string.format('%s 별칭을 부여할 수 없습니다.', name_with(model:name(), '은', '는')))
        return true
    end

    if rename_price > 0 then
        local money = me:money()
        if rename_price > money then
            npc:chat('돈이 모자랍니다.')
            return true
        end
    end

    local unicode = CP949(to)
    if #unicode < 4 then
        npc:chat('이름이 너무 짧습니다.')
        return true
    end
    
    if #unicode > 32 then
        npc:chat('이름이 너무 깁니다.')
        return true
    end
    
    if assert_korean(unicode) == false then
        npc:chat('그렇게 바꿀 수 없습니다.')
        return true
    end

    if rename_price > 0 then
        me:money(me:money() - rename_price)
    end
    weapon:rename(to)
    npc:chat(string.format('%s의 이름을 %s 변경했습니다.', from, name_with(to, '으로', '로')))
    return true
end

function npc_deposited_money(me, npc)

    local deposited = me:deposited_money()
    if deposited == 0 then
        npc:chat('맡긴 돈이 없습니다.')
    else
        npc:chat(string.format('금전 %d전을 맡아두고 있습니다.', deposited))
    end
    return true
end

function npc_hold_money(me, npc, money)

    local my_money = me:money()
    if money == nil then
        money = my_money
    end

    if money == 0 then
        return true
    end

    if money > my_money then
        npc:chat('돈이 모자랍니다.')
        return true
    end

    local deposited = me:deposited_money()
    local capacity = 0xFFFFFFFF - deposited
    if money > capacity then
        npc:chat('더 이상 맡길 수 없습니다.')
        return true
    end

    me:money(my_money - money)
    me:deposited_money(deposited + money)
    npc:chat(string.format('금전 %d전을 맡았습니다.', money))
end

function npc_return_money(me, npc, money)

    local deposited = me:deposited_money()
    if money == nil then
        money = deposited
    end

    if money == 0 then
        return true
    end

    if deposited == 0 then
        npc:chat('맡아둔 돈이 없습니다.')
        return true
    end

    if money > deposited then
        npc:chat('그만큼 맡기지 않았습니다.')
        return true
    end

    local capacity = 0xFFFFFFFF - me:money()
    if money > capacity then
        npc:chat('소지금이 너무 많습니다.')
        return false
    end

    me:deposited_money(deposited - money)
    me:money(me:money() + money)
    npc:chat(string.format('금전 %d전을 돌려드렸습니다.', money))
end

function npc_repair(me, npc, name)
    local all = (name == '')
    local items = {}
    if not all then
        local model = name2item(name)
        if model == nil then
            npc:chat('뭘 고쳐줘?')
            return true
        end

        if not model:attr(ITEM_ATTRIBUTE_EQUIPMENT) then
            npc:chat('뭘 고쳐줘?')
            return true
        end

        local item = me:item(name)
        if item == nil then
            npc:chat('가지고 있지 않은데요')
            return true
        end

        if model:repair_price() == nil then
            npc:chat(string.format('%s 고칠 수 없습니다.', name_with(name)))
            return true
        end

        if model:durability() == item:durability() then
            npc:chat('이미 고쳐져 있습니다.')
            return true
        end

        table.insert(items, item)
    else
        for slot, item in pairs(me:items()) do
            local model = item:model()
            if model:attr(ITEM_ATTRIBUTE_EQUIPMENT) and model:repair_price() ~= nil and model:durability() > item:durability() then
                table.insert(items, item)
            end
        end

        for parts, item in pairs(me:equipments()) do
            local model = item:model()
            if model:repair_price() ~= nil and model:durability() > item:durability() then
                table.insert(items, item)
            end
        end
    end

    if #items == 0 then
        npc:chat('고칠 물건이 없습니다.')
        return true
    end

    local price = 0
    for _, item in pairs(items) do
        local model = item:model()
        price = price + (model:repair_price() * (model:durability() - item:durability()))
    end
    
    price = math.floor(price)
    local money = me:money()
    if price > money then
        npc:chat('돈이 모자랍니다.')
        return true
    end

    for _, item in pairs(items) do
        local model = item:model()
        item:durability(model:durability())
    end

    if price == 0 then
        npc:chat('거의 새거라 그냥 고쳐드렸습니다. 잘 쓰세요.')
    else
        npc:chat(string.format('고치는데 %s전이 들었습니다.', price))
        me:money(money - price)
    end
end

function npc_hold_item(me, npc, name, count)
    local model = name2item(name)
    if model == nil then
        npc:chat('뭘 맡아줘?')
        return true
    end

    local item = me:item(name)
    if item == nil then
        npc:chat('가지고 있지 않은데요')
        return true
    end

    if model:deposit_price() == nil then
        npc:chat(string.format('%s 맡을 수 없습니다.', name_with(name, '은', '는')))
        return true
    end

    if count == nil then
        count = item:count()
    end

    if model:attr(ITEM_ATTRIBUTE_BUNDLE) then
        if count > item:count() then
            npc:chat('갯수가 모자라는데요?')
            return true
        end
    else
        count = 1
    end

    if model:deposit_price() > me:money() then
        npc:chat('돈이 모자랍니다.')
        return true
    end

    me:deposit_item(item, count)
    me:money(me:money() - model:deposit_price())
    if count > 1 then
        npc:chat(string.format('%s %d개 맡았습니다.', name_with(name, '을', '를'), count))
    else
        npc:chat(string.format('%s 맡았습니다.', name_with(name, '을', '를')))
    end

    return true
end

function npc_return_item(me, npc, name, count)
    local model = name2item(name)
    if model == nil then
        npc:chat('뭘 돌려줘?')
        return true
    end

    local item = me:deposited_item(name)
    if item == nil then
        npc:chat('그런 물품은 맡아두고 있지 않습니다.')
        return true
    end

    if count == nil then
        count = item:count()
    end

    if model:attr(ITEM_ATTRIBUTE_BUNDLE) then
        if count > item:count() then
            npc:chat('그만큼 맡고 있지 않습니다.')
            return true
        end
    else
        count = 1
    end

    local exists = me:item(name)
    if exists ~= nil and exists:count() + count > model:capacity() then
        npc:chat('더 이상 가질 수 없습니다.')
        return true
    end

    if me:withdraw_item(item, count) == nil then
        npc:chat('공간이 부족합니다.')
        return true
    end

    if count > 1 then
        npc:chat(string.format('%s %d개 돌려드렸습니다.', name_with(name, '을', '를'), count))
    else
        npc:chat(string.format('%s 돌려드렸습니다.', name_with(name, '을', '를')))
    end

    return true
end

function npc_sell_item(me, npc, name, count)
    local model = name2item(name)
    if model == nil then
        return true
    end

    local is_bundle = model:attr(ITEM_ATTRIBUTE_BUNDLE)
    if not is_bundle then
        count = 1
    end

    local price = npc:model():sell_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 팝니다.')
        return false
    end

    price = price * count
    local money = me:money()
    if price > money then
        npc:chat('돈이 모자랍니다.')
        return false
    end

    if is_bundle then
        local exist_count = 0
        local exist = me:item(name)
        if exist ~= nil then
            exist_count = exist:count()
        end

        if exist_count + count > model:capacity() then
            npc:chat('더 이상 가질 수 없습니다.')
            return false
        end
    end

    if me:mkitem(name, count) == nil then
        npc:chat('공간이 부족합니다.')
        return false
    end

    me:money(money - price)
    local message = nil
    if count > 1 then
        npc:chat(string.format('%s %d개를 %d전에 팔았습니다.', name, count, price))
    else
        npc:chat(string.format('%s %d전에 팔았습니다.', name_with(name), price))
    end
    return true
end

function npc_buy_item(me, npc, name, count)
    if count == 0 then
        return true
    end

    local model = name2item(name)
    if model == nil then
        return true
    end
    local is_bundle = model:attr(ITEM_ATTRIBUTE_BUNDLE)

    local price = npc:model():buy_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 삽니다.')
        return false
    end


    local slots = {}
    for slot, item in pairs(me:items()) do
        if item:model():name() == name then
            table.insert(slots, slot)
            if is_bundle then
                break
            else
                if count ~= nil and #slots > count then
                    break
                end
            end
        end
    end

    if #slots == 0 then
        npc:chat('가지고 있지 않습니다.')
        return false
    end

    if is_bundle then
        local item = me:item(slots[1] - 1)
        if count == nil then
            count = item:count()
        end

        if count > item:count() then
            npc:chat('그만큼 가지고 있지 않습니다.')
            return false
        end
    else
        if count == nil then
            count = #slots
        end

        if count > #slots then
            npc:chat('그만큼 가지고 있지 않습니다.')
            return false
        end
    end

    price = price * count
    local capacity = 0xFFFFFFFF - me:money()
    if price > capacity then
        npc:chat('돈이 너무 많습니다.')
        return false
    end

    if is_bundle then
        me:rmitem(slots[1], count, ITEM_DELETE_TYPE_SELL)
    else
        for _, slot in pairs(slots) do
            me:rmitem(slot, 1, ITEM_DELETE_TYPE_SELL)
        end
    end
    me:money(me:money() + price)
    if count > 1 then
        npc:chat(string.format('%s %d개를 %d전에 샀습니다.', name, count, price))
    else
        npc:chat(string.format('%s %d전에 샀습니다.', name_with(name), price))
    end
    return true
end

function npc_sell_item_list(me, npc)
    local limit = 3
    local names = {}
    local overflow = false
    for _, sell in pairs(npc:model():sell()) do
        for _, pair in pairs(pursuit_sell(sell)) do
            local item, price = table.unpack(pair)
            if #names < limit then
                table.insert(names, item:name())
            else
                overflow = true
                break
            end
        end
    end

    local message = table.concat(names, ", ")
    if overflow then
        message = message .. ' 등 여러가지를'
    else
        message = name_with(message)
    end
    message = message .. ' 판매하고 있습니다.'
    npc:chat(message)
    return false
end

function npc_buy_item_list(me, npc)
    local limit = 3
    local names = {}
    local overflow = false
    for _, pair in pairs(pursuit_buy(npc:model():buy())) do
        local item, price = table.unpack(pair)
        if #names < limit then
            table.insert(names, item:name())
        else
            overflow = true
            break
        end
    end

    local message = table.concat(names, ", ")
    if overflow then
        message = message .. ' 등 여러가지를'
    else
        message = name_with(message)
    end
    message = message .. ' 구입하고 있습니다.'
    npc:chat(message)
    return false
end

function npc_sell_item_price(me, npc, name)
    local price = npc:model():sell_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 팝니다.')
        return false
    end

    npc:chat(string.format('%s %d전에 팔고 있습니다.', name_with(name, '은', '는'), price))
    return false
end

function npc_buy_item_price(me, npc, name)
    local price = npc:model():buy_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 삽니다.')
        return false
    end

    npc:chat(string.format('%s %d전에 사고 있습니다.', name_with(name, '은', '는'), price))
    return false
end

function npc_buy_dialog(me, npc)
    local purchase_list = {}
    for i, pair in pairs(pursuit_buy(npc:buy())) do
        local item, price = table.unpack(pair)
        purchase_list[item:name()] = price
    end

    local slots = {}
    local items = {}
    local my_items = me:items()
    for slot, item in pairs(my_items) do
        local model = item:model()
        local name = model:name()
        if purchase_list[model:name()] ~= nil then
            table.insert(slots, slot)
            items[slot] = item
        end
    end

::ROUTINE0000::
    local slot = npc:slot(me, '뭘 팔래요?', slots)
    if slot == nil then
        return DIALOG_RESULT_NEXT
    end

    local item = items[slot]
    local model = item:model()
    local price = purchase_list[model:name()]
    local count = 1
    if model:attr(ITEM_ATTRIBUTE_BUNDLE) then
        count = npc:input(me, '몇개나 파시겠어요?')
        if count == nil then
            goto ROUTINE0000
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return npc:dialog(me, '갯수가 올바르지 않습니다.', false, true)
        end

        if count > item:count() then
            return npc:dialog(me, '그만큼 가지고 있지 않습니다.', false, true)
        end
    end
    price = price * count

    local message = nil
    if count > 1 then
        message = string.format('%s %d개를 %d전에 파시겠습니까?', model:name(), count, price)
    else
        message = string.format('%s %d전에 파시겠습니까?', name_with(model:name()), price)
    end
    if npc:menu(me, message, {'네', '아니오'}) == 0 then
        me:money(me:money() + price)
        me:rmitem(slot, count, ITEM_DELETE_TYPE_SELL)
    end
    return DIALOG_RESULT_NEXT
end

function npc_sell_dialog(me, npc)
    local pursuit = npc:sell()
    if #pursuit > 1 then
        local menu = {}
        for _, sell in pairs(pursuit) do
            table.insert(menu, pursuit_sell_name(sell))
        end
        local selected, button = npc:list(me, '무엇을 사시겠어요?', menu, true)
        if selected == nil then
            if button == DIALOG_RESULT_QUIT then
                return DIALOG_RESULT_QUIT
            else
                return DIALOG_RESULT_NEXT
            end
        end
        pursuit = pursuit[selected+1]
    else
        pursuit = pursuit[1]
    end

    local list = pursuit_sell(pursuit)
::ROUTINE0002::
    local selected = npc:item(me, '제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', list)
    if selected == nil then
        return DIALOG_RESULT_NEXT
    end

    local count = 1
    local item = name2item(selected)
    local is_bundle = item:attr(ITEM_ATTRIBUTE_BUNDLE)
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

    local price = pursuit_sell_price(pursuit, selected)
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
        if model:attr(ITEM_ATTRIBUTE_EQUIPMENT) and model:repair_price() ~= nil then
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



function npc_repair_dialog(me, npc)
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

    if price == 0 then
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



function npc_repair_all_dialog(me, npc)
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

    if price == 0 then
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



function npc_hold_money_dialog(me, npc)
    local count = npc:input(me, '얼마나 맡아드릴까요?')
    if count == nil then
        return DIALOG_RESULT_NEXT
    end

    count = tonumber(count)
    if count == nil or count <= 0 then
        return npc:dialog(me, '금액이 올바르지 않습니다.', false, true)
    end

    local current_money = me:money()
    if count > current_money then
       return npc:dialog(me, '돈이 모자랍니다.', false, true) 
    end

    local deposited_money = me:deposited_money()
    local capacity = 0xFFFFFFFF - deposited_money
    if count > capacity then
        return npc:dialog(me, '그만큼 맡아드릴 수 없습니다.', false, true)
    end

    me:deposited_money(deposited_money + count)
    me:money(current_money - count)
    return npc:dialog(me, string.format('금전 %d전을 맡았습니다.', count), false, true)
end



function npc_hold_item_dialog(me, npc)
    local slots = {}
    local items = {}
    local my_items = me:items()
    for slot, item in pairs(my_items) do
        local model = item:model()
        if model:deposit_price() ~= nil then
            table.insert(slots, slot)
            items[slot] = item
        end
    end

    local slot = npc:slot(me, '무엇을 맡기시겠습니까?', slots)
    local item = items[slot]
    local model = item:model()
    local count = 1
    if model:attr(ITEM_ATTRIBUTE_BUNDLE) then
        count = npc:input(me, '얼마나 맡아드릴까요?')
        if count == nil then
            return DIALOG_RESULT_NEXT
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return npc:dialog(me, '수량이 올바르지 않습니다.', false, true)
        end

        if item:count() < count then
            return npc:dialog(me, '그만큼 가지고 있지 않습니다.', false, true)
        end

        local deposited_item = me:deposited_item(model:name())
        if deposited_item ~= nil then
            local deposited_count = deposited_item:count()
            local capacity = 0xFFFF - deposited_count
            if count > capacity then
                return npc:dialog(me, '더 이상 맡길 수 없습니다.', false, true)
            end
        end
    end

    local deposit_price = model:deposit_price()
    if deposit_price > 0 then
        local selected = npc:menu(me, string.format('맡기는데 %d전이 필요합니다. 맡기시겠습니까?', deposit_price), {'네', '아니오'})
        if selected == 0 then
            if me:money() < deposit_price then
                return npc:dialog(me, '돈이 모자랍니다.')
            else
                me:money(me:money() - deposit_price)
            end
        else
            return DIALOG_RESULT_NEXT
        end
    end

    me:deposit_item(item, count)
    if model:attr(ITEM_ATTRIBUTE_BUNDLE) then
        return npc:dialog(me, string.format('%s %d개를 맡았습니다.', model:name(), count), false, true)
    else
        return npc:dialog(me, string.format('%s 맡았습니다.', name_with(model:name()), count), false, true)
    end
end



function npc_return_money_dialog(me, npc)
    local deposited_money = me:deposited_money()
    if deposited_money <= 0 then
        return npc:dialog(me, '돈을 보관하고 있지 않습니다.', false, true)
    end

    local count = npc:input(me, string.format('제가 %d전을 보관하고 있습니다. 얼마나 돌려드릴까요?', deposited_money))
    if count == nil then
        return DIALOG_RESULT_NEXT
    end

    count = tonumber(count)
    if count == nil or count <= 0 then
        return npc:dialog(me, '금액이 올바르지 않습니다.', false, true)
    end

    if count > deposited_money then
        return npc:dialog(me, '그만큼 보관하고 있지 않습니다.', false, true)
    end

    local current_money = me:money()
    local capacity = 0xFFFFFFFF - current_money
    if count > capacity then
       return npc:dialog(me, '가진 돈이 너무 많습니다.', false, true)
    end

    me:deposited_money(deposited_money - count)
    me:money(current_money + count)
    return npc:dialog(me, string.format('금전 %d전을 돌려드렸습니다.', count), false, true)
end



function npc_return_item_dialog(me, npc)
    local list = {}
    for _, deposited_item in pairs(me:deposited_item()) do
        local model = deposited_item:model()
        local count = deposited_item:count()
        table.insert(list, {model, count})
    end

    local selected = npc:item(me, '제가 맡고 있는 물건들입니다. 무엇을 찾으시겠습니까?', list)
    if selected == nil then
        return DIALOG_RESULT_NEXT
    end

    local deposited_item = me:deposited_item(selected)
    local model = deposited_item:model()
    local count = 1
    if model:attr(ITEM_ATTRIBUTE_BUNDLE)  then
        if deposited_item:count() > 1 then
            count = npc:input(me, '얼마나 돌려드릴까요?')
            if count == nil then
                return DIALOG_RESULT_NEXT
            end
        else
            count = 1
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return npc:dialog(me, '수량이 올바르지 않습니다.', false, true)
        end

        if deposited_item:count() < count then
            return npc:dialog(me, '그만큼 맡고 있지 않습니다.', false, true)
        end

        local exists = me:item(model)
        if exists ~= nil and exists:count() + count > model:capacity() then
            return npc:dialog(me, '더 이상 가질 수 없습니다.', false, true)
        end
    end
    
    if me:withdraw_item(deposited_item, count) == nil then
        return npc:dialog(me, '공간이 부족합니다.', false, true)
    end

    if model:attr(ITEM_ATTRIBUTE_BUNDLE) and count > 1 then
        return npc:dialog(me, string.format('%s %d개를 돌려드렸습니다.', model:name(), count), false, true)
    else
        return npc:dialog(me, string.format('%s 돌려드렸습니다.', name_with(model:name()), count), false, true)
    end
end

function npc_rename_weapon_dialog(me, npc)
    local slots = {}
    local items = {}
    for slot, item in pairs(me:items()) do
        local model = item:model()
        if model:attr(ITEM_ATTRIBUTE_WEAPON) and model:rename_price() ~= nil then
            table.insert(slots, slot)
            items[slot] = item
        end
    end

::RENAME_WEAPON_0001::
    local slot = npc:slot(me, '어떤 장비에 별칭을 부여하시겠어요?', slots)
    if slot == nil then
        return DIALOG_RESULT_NEXT
    end

    local weapon = items[slot]
::RENAME_WEAPON_0002::
    local name = npc:input(me, '어떤 이름을 붙이고 싶으세요?')
    if name == nil then
        goto RENAME_WEAPON_0001
    end

    local cp949 = CP949(name)
    if #cp949 < 4 then
        npc:dialog(me, '이름이 너무 짧습니다.', false, true)
        goto RENAME_WEAPON_0002
    end

    if #cp949 > 32 then
        npc:dialog(me, '이름이 너무 깁니다.', false, true)
        goto RENAME_WEAPON_0002
    end

    if not assert_korean(cp949) then
        npc:dialog(me, string.format('%s 사용할 수 없는 이름입니다.', name_with(name, '은', '는')), false, true)
        goto RENAME_WEAPON_0002
    end

    local price = weapon:model():rename_price()
    local money = me:money()
    if price > 0 then
        if npc:menu(me, string.format('바꾸는데 %d전이 필요합니다. 정말 바꾸시겠습니까?', price), {'네', '아니오'}) == 0 then
            if price > money then
                return npc:dialog(me, '돈이 부족합니다.', false, true)
            end

            me:money(money - price)
        else
            return DIALOG_RESULT_NEXT
        end
    end

    weapon:rename(name)
    return npc:dialog(me, string.format('%s의 이름을 %s 변경했습니다.', weapon:model():name(), name_with(name, '으로', '로')), false, true)
end

function NPC_BASIC_CLASS(me, npc, class, spells)
::NPC_BASIC_CLASS_000::
    local level = me:level()
    local class_name = class2name(class)
    if me:class() == class then
        local selected = npc:menu(me, '험난한 길을 걷는 수행자여, 무슨 일로 저를 찾으셨소?', {'마법 알아보기', '마법 배우기', '마법 지우기', '칭호 받기'})
        if selected == 0 then
            local learned_spells = {}
            for _, spell in pairs(me:spells()) do
                learned_spells[spell:model():name()] = spell
            end

            local preview = {}
            for name, spell in pairs(spells) do
                if learned_spells[name] == nil and spell.level < level + 5 then
                    table.insert(preview, name)
                end
            end

            if #preview == 0 then
                if npc:dialog(me, '내가 알려줄 마법이 없구나..', false, true) == DIALOG_RESULT_QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            table.sort(preview, function(a, b) return spells[a].level < spells[b].level end)
            selected = npc:list(me, '자네 수준이라면 이런 마법들을 알아볼 수 있겠군', preview)
            if selected == nil then
                return
            end

            local name = preview[selected+1]
            local spell = spells[name]
            if npc:dialog(me, string.format('%s %s', name_with(name, '은', '는'), spell.desc), false, true) == DIALOG_RESULT_QUIT then
                return
            end

            local material = {}
            for _, m in pairs(spell.material) do
                local name = m['name']
                local count = m['count']
                if name == '금전' then
                    table.insert(material, string.format('%s %d전', name, count))
                else
                    table.insert(material, string.format('%s %d개', name, count))
                end
            end

            if npc:dialog(me, string.format('%s 배우기 위해서는 %s를 바쳐야 하네', name_with(name), table.concat(material, ', ')), false, true) == DIALOG_RESULT_QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        elseif selected == 1 then
            local learned_spells = {}
            for _, spell in pairs(me:spells()) do
                learned_spells[spell:model():name()] = spell
            end

            local preview = {}
            for name, spell in pairs(spells) do
                if learned_spells[name] == nil and spell.level <= level then
                    table.insert(preview, name)
                end
            end

            if #preview == 0 then
                if npc:dialog(me, '내가 알려줄 마법이 없구나..', false, true) == DIALOG_RESULT_QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            table.sort(preview, function(a, b) return spells[a].level < spells[b].level end)
            selected = npc:list(me, '자네 수준이라면 이런 마법들을 배울 수 있겠군', preview)
            if selected == nil then
                return
            end

            local name = preview[selected+1]
            local spell = spells[name]
            if npc:dialog(me, string.format('%s %s', name_with(name, '은', '는'), spell.desc), false, true) == DIALOG_RESULT_QUIT then
                return
            end
            
            local material = {}
            for _, m in pairs(spell.material) do
                if m.name == '금전' then
                    table.insert(material, string.format('%s %d전', m.name, m.count))
                else
                    table.insert(material, string.format('%s %d개', m.name, m.count))
                end
            end
            
            selected = npc:list(me, string.format('%s 배우기 위해서는 %s를 바쳐야 하네. 배우겠느냐?', name_with(name), table.concat(material, ', ')), {'예', '아니오'})
            if selected == nil then
                return
            end

            if selected == 1 then
                if npc:dialog(me, '이 모든 것은 네 탓이니, 다음에 이 곳에 올 때는 더 굳은 각오를 가지고 오도록 하거라.', false, true) == DIALOG_RESULT_QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            local enough = true
            for _, m in pairs(spell.material) do
                if m.name == '금전' then
                    if me:money() < m.count then
                        enough = false
                        break
                    end
                else
                    local item = me:item(m.name)
                    if item == nil or item:count() < m.count then
                        enough = false
                        break
                    end
                end
            end

            if not enough then
                if npc:dialog(me, '필요한 것들을 구해보도록 하게. 자네라면 할 수 있겠지?', false, true) == DIALOG_RESULT_QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            for _, m in pairs(spell.material) do
                if m.name == '금전' then
                    me:money(me:money() - m.count)
                else
                    me:rmitem(m.name, m.count)
                end
            end
            me:mkspell(name)
            if npc:dialog(me, '배움의 길은 끝이 없으니 더더욱 노력하는 자세를 갖도록 하거라', false, true) == DIALOG_RESULT_QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        elseif selected == 2 then
            local learned_spells = {}
            for _, spell in pairs(me:spells()) do
                table.insert(learned_spells, spell:model():name())
            end
            selected = npc:list(me, '지금 네가 지울 수 있는 마법은 다음과 같단다. 다시 한 번 심사 숙고 하고 지우도록 하여라.', learned_spells)
            if selected == nil then
                return
            end

            me:rmspell(learned_spells[selected+1])
            if npc:dialog(me, '배움의 길은 끝이 없으니 더더욱 노력하는 자세를 갖도록 하거라', false, true) == DIALOG_RESULT_QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        elseif selected == 3 then
            local title = npc:input(me, '네 정성이 갸륵하니... 그래, 무슨 칭호를 받고 싶으냐?', '받고싶은 칭호는', '입니다.', 10, true)
            if title == DIALOG_RESULT_QUIT then
                return
            end

            selected = npc:list(me, '그 칭호로 바꾸려면 금전 5000전을 바쳐야 하느니라. 네 소원을 이루겠느냐?', {'예', '아니오'})
            if selected == nil then
                return
            end

            if me:money() < 5000 then
                if npc:dialog(me, '예끼 이놈! 감히 돈도 없으면서 날 찾아와?', false, true) == DIALOG_RESULT_QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            if npc:dialog(me, '그럼, 너의 소원을 들어주겠노라', false, true) == DIALOG_RESULT_QUIT then
                return
            end
            me:money(me:money() - 5000)
            me:title(title)
            if npc:dialog(me, '칭호를 받았으니, 자신감을 가 지고 다니게나...', false, true) == DIALOG_RESULT_QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        end
    else
        local button = npc:menu(me, '험난한 길을 걷는 수행자여, 무슨 일로 저를 찾으셨소?', {class_name .. '직업가지기'})
        if button == 0 then
            if me:class() ~= CLASS_NONE then
                npc:dialog(me, '이미 직업이 있지 않느냐? 한번 선택한 직업은 바꿀 수 없느니라.')
                return
            end

            if level < 5 then
                npc:dialog(me, '아직 너의 정성이 부족하니 다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를 찾아오도록 하여라.')
            end
::NPC_BASIC_CLASS_001::
            button = npc:dialog(me, class_name .. '의 길을 가려면, 몇 가지 맹세를 해야하느니.', false, true)
            if button == DIALOG_RESULT_QUIT then
                return
            end

::NPC_BASIC_CLASS_002::
            local selected, button = npc:list(me, '첫째로, 하늘에서 굽어보고 계신 천제(天帝) 앞에 복종을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT_QUIT then
                return
            end
            if button == DIALOG_RESULT_PREV then
                goto NPC_BASIC_CLASS_001
            end
::NPC_BASIC_CLASS_003::
            selected, button = npc:list(me, '둘째로, 험난한 ' .. class_name .. '수련의 길에 너의 평생을 바칠 것을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT_QUIT then
                return
            end
            if button == DIALOG_RESULT_PREV then
                goto NPC_BASIC_CLASS_002
            end
::NPC_BASIC_CLASS_004::
            selected, button = npc:list(me, '셋째로, 불의를 보고 그냥 지나치지 않을 것을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT_QUIT then
                return
            end
            if button == DIALOG_RESULT_PREV then
                goto NPC_BASIC_CLASS_003
            end
::NPC_BASIC_CLASS_005::
            selected, button = npc:list(me, '훌륭하군. 그렇다면, 지금까지의 맹세를 증명하기 위해 도토리를 10개 바치거라.', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            elseif button == DIALOG_RESULT_QUIT then
                return
            elseif button == DIALOG_RESULT_PREV then
                goto NPC_BASIC_CLASS_004
            else
                local item = me:item('도토리')
                if item == nil then
                    goto NPC_BASIC_CLASS_STOP
                elseif item:count() < 10 then
                    goto NPC_BASIC_CLASS_STOP
                else
                    me:rmitem('도토리', 10)
                    me:class(class)
                    if npc:dialog(me, '자네는 이제 ' .. class_name .. '으로써의 끝없는 길을 가게 되었네.', false, true) == DIALOG_RESULT_QUIT then
                        return
                    end

                    if npc:dialog(me, '그 길은 멀고도 험난할 테니, 마음을 굳건히 하거라.', false, true) == DIALOG_RESULT_QUIT then
                        return
                    end
                    goto NPC_BASIC_CLASS_000
                end
            end

::NPC_BASIC_CLASS_STOP::
            npc:dialog(me, '아직 너의 정성이 부족하니 다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를 찾아오도록 하여라.')
        end
    end
end