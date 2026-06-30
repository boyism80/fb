local M = {}

function M.revive(me, npc, discourteous)
    if me:state() ~= STATE.GHOST then
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
        me:state(STATE.NORMAL)
        me:hp(50)
    end
    return true
end

function M.appreciate(me, npc)
    if me:state() == STATE.GHOST then
        return true
    end

    if me:hp() > 100 then
        return true
    end

    npc:chat('예의 바른 놈. 잘 살거라.')
    me:hp(me:maxhp())
    return true
end

function M.store_item_count(me, npc, name)
    local model = name2item(name)
    if model == nil then
        return true
    end

    local item = me:stored_item(model)
    if item == nil then
        npc:chat('그런 물건은 맡고 있지 않습니다.')
        return true
    end

    npc:chat(string.format('%s %d개 맡고 있습니다.', name_with(name), item:count()))
    return true
end

function M.store_item_list(me, npc)
    local items = me:stored_item()
    local count = #items
    if count == 0 then
        npc:chat('맡긴 물건이 없습니다.')
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

function M.rename_weapon(me, npc, from, to)
    local model = name2item(from)
    if model == nil then
        return true
    end

    if model:attr(ITEM_ATTRIBUTE.WEAPON) == false then
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

function M.deposited_money(me, npc)

    local stored = me:deposited_money()
    if stored == 0 then
        npc:chat('맡긴 돈이 없습니다.')
    else
        npc:chat(string.format('금전 %d전을 맡아두고 있습니다.', stored))
    end
    return true
end

function M.deposit_money(me, npc, money)
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
    return true
end

function M.withdraw_money(me, npc, money)
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
    return true
end

function M.repair(me, npc, name)
    local all = (name == nil)
    local items = {}
    if not all then
        local model = name2item(name)
        if model == nil then
            npc:chat('뭘 고쳐줘?')
            return true
        end

        if not model:attr(ITEM_ATTRIBUTE.EQUIPMENT) then
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
            if model:attr(ITEM_ATTRIBUTE.EQUIPMENT) and model:repair_price() ~= nil and model:durability() > item:durability() then
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
    return true
end

function M.store_item(me, npc, name, count)
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

    if model:storage_fee() == nil then
        npc:chat(string.format('%s 맡을 수 없습니다.', name_with(name, '은', '는')))
        return true
    end

    if count == nil then
        count = item:count()
    end

    if model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        if count > item:count() then
            npc:chat('갯수가 모자라는데요?')
            return true
        end
    else
        count = 1
    end

    if model:storage_fee() > me:money() then
        npc:chat('돈이 모자랍니다.')
        return true
    end

    me:store_item(item, count)
    me:money(me:money() - model:storage_fee())
    if count > 1 then
        npc:chat(string.format('%s %d개 맡았습니다.', name_with(name, '을', '를'), count))
    else
        npc:chat(string.format('%s 맡았습니다.', name_with(name, '을', '를')))
    end

    return true
end

function M.retrieve_item(me, npc, name, count)
    local model = name2item(name)
    if model == nil then
        npc:chat('뭘 돌려줘?')
        return true
    end

    local item = me:stored_item(name)
    if item == nil then
        npc:chat('그런 물품은 맡아두고 있지 않습니다.')
        return true
    end

    if count == nil then
        count = item:count()
    end

    if model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        if count > item:count() then
            npc:chat('그만큼 맡고 있지 않습니다.')
            return true
        end
    else
        count = 1
    end

    local exists_count = 0
    local exists = me:item(name)
    if exists ~= nil then
        exists_count = exists:count()
    end
    if exists_count + count > model:capacity() then
        npc:chat('더 이상 가질 수 없습니다.')
        return true
    end

    if me:retrieve_item(item, count) == nil then
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

function M.sell_item(me, npc, name, count)
    local model = name2item(name)
    if model == nil then
        return true
    end

    local is_bundle = model:attr(ITEM_ATTRIBUTE.BUNDLE)
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

function M.buy_item(me, npc, name, count)
    if count == 0 then
        return true
    end

    local model = name2item(name)
    if model == nil then
        return true
    end
    local is_bundle = model:attr(ITEM_ATTRIBUTE.BUNDLE)

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
        me:rmitem(slots[1], count, ITEM_DELETE_TYPE.SELL)
    else
        for _, slot in pairs(slots) do
            me:rmitem(slot, 1, ITEM_DELETE_TYPE.SELL)
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

function M.sell_item_list(me, npc)
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

    if #names == 0 then
        return false
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

function M.buy_item_list(me, npc)
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

function M.sell_item_price(me, npc, name)
    local price = npc:model():sell_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 팝니다.')
        return false
    end

    npc:chat(string.format('%s %d전에 팔고 있습니다.', name_with(name, '은', '는'), price))
    return false
end

function M.buy_item_price(me, npc, name)
    local price = npc:model():buy_price(name)
    if price == nil then
        npc:chat('그런 물건은 안 삽니다.')
        return false
    end

    npc:chat(string.format('%s %d전에 사고 있습니다.', name_with(name, '은', '는'), price))
    return false
end

function M.count_item_by_name(me, item_name)
    local total = 0
    local items = me:items()
    if items then
        for _, item in pairs(items) do
            if item:model():name() == item_name then
                total = total + item:count()
            end
        end
    end
    return total
end

local function shop_continue(result)
    if result == nil then
        return false
    end
    if result == true then
        return true
    end
    return result == DIALOG_RESULT.NEXT
end

function M.shop(me, npc, config)
    local greeting = config.greeting or '안녕하세요. 무엇을 도와드릴까요?'
    local menu = config.menu
    if menu == nil then
        return
    end

    while true do
        local labels = {}
        for i = 1, #menu do
            labels[i] = menu[i][1]
        end

        local selected = me:list(npc, greeting, labels)
        if selected == nil then
            return
        end

        local entry = menu[selected + 1]
        if entry == nil then
            return
        end

        if not shop_continue(entry[2](me, npc)) then
            return
        end
    end
end

function M.show_buy_menu(me, npc)
    local purchase_list = {}
    for i, pair in pairs(pursuit_buy(npc:model():buy())) do
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

::NPC_BUY_DIALOG_000::
    local slot = me:slot(npc, '뭘 팔래요?', slots)
    if slot == nil then
        return DIALOG_RESULT.NEXT
    end

    local item = items[slot]
    local model = item:model()
    local price = purchase_list[model:name()]
    local count = 1
    if model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        count = me:input(npc, '몇개나 파시겠어요?')
        if count == nil then
            goto NPC_BUY_DIALOG_000
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return me:dialog(npc, '갯수가 올바르지 않습니다.', false, true)
        end

        if count > item:count() then
            return me:dialog(npc, '그만큼 가지고 있지 않습니다.', false, true)
        end
    end
    price = price * count

    local message = nil
    if count > 1 then
        message = string.format('%s %d개를 %d전에 파시겠습니까?', model:name(), count, price)
    else
        message = string.format('%s %d전에 파시겠습니까?', name_with(model:name()), price)
    end
    if me:menu(npc, message, {'네', '아니오'}) == 0 then
        me:money(me:money() + price)
        me:rmitem(slot, count, ITEM_DELETE_TYPE.SELL)
    end
    return DIALOG_RESULT.NEXT
end

local function show_sell_catalog(me, npc, sell_id)
    local pursuit = sell_id
    if pursuit == nil then
        local sells = npc:model():sell()
        if #sells == 0 then
            pursuit = nil
        elseif #sells == 1 then
            pursuit = sells[1]
        else
            return DIALOG_RESULT.NEXT
        end
    end

    if pursuit == nil then
        return DIALOG_RESULT.NEXT
    end

    local list = pursuit_sell(pursuit)
::NPC_SELL_DIALOG_000::
    local selected = me:item(npc, '제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.', list)
    if selected == nil then
        return DIALOG_RESULT.NEXT
    end

    local count = 1
    local item = name2item(selected)
    if item == nil then
        return me:dialog(npc, 'This player is a hacker.')
    end

    local is_bundle = item:attr(ITEM_ATTRIBUTE.BUNDLE)
    if is_bundle then
        count = me:input(npc, '몇개나 사시겠어요?')
        if count == nil then
            goto NPC_SELL_DIALOG_000
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return me:dialog(npc, '갯수가 올바르지 않습니다.', false, true)
        end
    end

    local price = pursuit_sell_price(pursuit, selected)
    if price == nil then
        return me:dialog(npc, '알 수 없는 에러')
    else
        price = price * count
    end

    local money = me:money()
    if price > money then
        return me:dialog(npc, '돈이 모자랍니다.', false, true)
    end

    local exist_count = 0
    local exist = me:item(selected)
    if exist ~= nil then
        exist_count = exist:count()
    end

    if is_bundle and exist_count + count > item:capacity() then
        return me:dialog(npc, '더 이상 가질 수 없습니다.', false, true)
    end
    
    if me:mkitem(selected, count) == nil then
        return me:dialog(npc, '공간이 부족합니다.', false, true)
    else
        me:money(money - price)
        local message = nil
        if count > 1 then
            message = string.format('%s %d개를 %d전에 팔았습니다.', selected, count, price)
        else
            message = string.format('%s %d전에 팔았습니다.', name_with(selected), price)
        end
        return me:dialog(npc, message, false, true)
    end
end

function M.show_sell_menu(me, npc, categories)
    if categories ~= nil then
        local labels = {}
        for i = 1, #categories do
            labels[i] = categories[i][1]
        end

        local selected, button = me:list(npc, '무엇을 사시겠어요?', labels, true)
        if selected == nil then
            if button == DIALOG_RESULT.QUIT then
                return DIALOG_RESULT.QUIT
            end
            return DIALOG_RESULT.NEXT
        end

        return show_sell_catalog(me, npc, categories[selected + 1][2])
    end

    return show_sell_catalog(me, npc)
end

function M.repairable_slots(me)
    local items = me:items()
    local slots = {}
    local count = 0

    for slot, item in pairs(items) do
        local model = item:model()
        if model:attr(ITEM_ATTRIBUTE.EQUIPMENT) and model:repair_price() ~= nil then
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



function M.show_repair_menu(me, npc)
    local items = M.repairable_slots(me)
    if items == nil then
        return me:dialog(npc, '고칠 물건이 없는데요', false, true)
    end

    local slots = {}
    for slot, item in pairs(items) do
        table.insert(slots, slot)
    end

    local selected = me:slot(npc, '무엇을 고치시겠습니까?', slots)
    if selected == nil then
        return DIALOG_RESULT.NEXT
    end

    local item = items[selected]
    local model = item:model()
    local price = math.floor(model:repair_price() * (model:durability() - item:durability()))

    if price == 0 then
        item:durability(model:durability())
        return me:dialog(npc, '거의 새거라 그냥 고쳐드렸습니다. 잘 쓰세요')
    else
        if me:menu(npc, string.format('고치는데 %d전이 필요합니다. 정말 수리하시겠습니까?', price), {'네', '아니오'}) == 0 then
            local money = me:money()
            if price > money then
                return me:dialog(npc, '돈이 모자랍니다.', false, true)
            else
                me:money(money - price)
                item:durability(model:durability())
                return DIALOG_RESULT.NEXT
            end
        else
            return DIALOG_RESULT.NEXT
        end
    end
end



function M.show_repair_all_menu(me, npc)
    local items = M.repairable_slots(me)
    if items == nil then
        return me:dialog(npc, '고칠 물건이 없는데요', false, true)
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
        return me:dialog(npc, '거의 새거라 그냥 고쳐드렸습니다. 잘 쓰세요')
    else
        local selected = me:menu(npc, string.format('모두 고치는데 %d전이 필요합니다. 고치시겠습니까?', price), {'네', '아니오'})
        if selected == 0 then
            local money = me:money()
            if money < price then
                return me:dialog(npc, '돈이 모자랍니다.')
            else
                for slot, item in pairs(items) do
                    local model = item:model()
                    item:durability(model:durability())
                end

                me:money(money - price)
                return me:dialog(npc, '모두 고쳤습니다.')
            end
        else
            return DIALOG_RESULT.NEXT
        end
    end
end



function M.show_hold_money_menu(me, npc)
    local count = me:input(npc, '얼마나 맡아드릴까요?')
    if count == nil then
        return DIALOG_RESULT.NEXT
    end

    count = tonumber(count)
    if count == nil or count <= 0 then
        return me:dialog(npc, '금액이 올바르지 않습니다.', false, true)
    end

    local current_money = me:money()
    if count > current_money then
       return me:dialog(npc, '돈이 모자랍니다.', false, true) 
    end

    local deposited_money = me:deposited_money()
    local capacity = 0xFFFFFFFF - deposited_money
    if count > capacity then
        return me:dialog(npc, '그만큼 맡아드릴 수 없습니다.', false, true)
    end

    me:deposited_money(deposited_money + count)
    me:money(current_money - count)
    return me:dialog(npc, string.format('금전 %d전을 맡았습니다.', count), false, true)
end



function M.show_hold_item_menu(me, npc)
    local slots = {}
    local items = {}
    local my_items = me:items()
    for slot, item in pairs(my_items) do
        local model = item:model()
        if model:storage_fee() ~= nil then
            table.insert(slots, slot)
            items[slot] = item
        end
    end

    local slot = me:slot(npc, '무엇을 맡기시겠습니까?', slots)
    local item = items[slot]
    local model = item:model()
    local count = 1
    if model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        count = me:input(npc, '얼마나 맡아드릴까요?')
        if count == nil then
            return DIALOG_RESULT.NEXT
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return me:dialog(npc, '수량이 올바르지 않습니다.', false, true)
        end

        if item:count() < count then
            return me:dialog(npc, '그만큼 가지고 있지 않습니다.', false, true)
        end

        local stored_item = me:stored_item(model:name())
        if stored_item ~= nil then
            local stored_count = stored_item:count()
            local capacity = 0xFFFF - stored_count
            if count > capacity then
                return me:dialog(npc, '더 이상 맡길 수 없습니다.', false, true)
            end
        end
    end

    local storage_fee = model:storage_fee()
    if storage_fee > 0 then
        local selected = me:menu(npc, string.format('맡기는데 %d전이 필요합니다. 맡기시겠습니까?', storage_fee), {'네', '아니오'})
        if selected == 0 then
            if me:money() < storage_fee then
                return me:dialog(npc, '돈이 모자랍니다.')
            else
                me:money(me:money() - storage_fee)
            end
        else
            return DIALOG_RESULT.NEXT
        end
    end

    me:store_item(item, count)
    if model:attr(ITEM_ATTRIBUTE.BUNDLE) then
        return me:dialog(npc, string.format('%s %d개를 맡았습니다.', model:name(), count), false, true)
    else
        return me:dialog(npc, string.format('%s 맡았습니다.', name_with(model:name()), count), false, true)
    end
end



function M.show_return_money_menu(me, npc)
    local deposited_money = me:deposited_money()
    if deposited_money <= 0 then
        return me:dialog(npc, '돈을 보관하고 있지 않습니다.', false, true)
    end

    local count = me:input(npc, string.format('제가 %d전을 보관하고 있습니다. 얼마나 돌려드릴까요?', deposited_money))
    if count == nil then
        return DIALOG_RESULT.NEXT
    end

    count = tonumber(count)
    if count == nil or count <= 0 then
        return me:dialog(npc, '금액이 올바르지 않습니다.', false, true)
    end

    if count > deposited_money then
        return me:dialog(npc, '그만큼 보관하고 있지 않습니다.', false, true)
    end

    local current_money = me:money()
    local capacity = 0xFFFFFFFF - current_money
    if count > capacity then
       return me:dialog(npc, '가진 돈이 너무 많습니다.', false, true)
    end

    me:deposited_money(deposited_money - count)
    me:money(current_money + count)
    return me:dialog(npc, string.format('금전 %d전을 돌려드렸습니다.', count), false, true)
end



function M.show_return_item_menu(me, npc)
    local list = {}
    for _, stored_item in pairs(me:stored_item()) do
        local model = stored_item:model()
        local count = stored_item:count()
        table.insert(list, {model, count})
    end

    local selected = me:item(npc, '제가 맡고 있는 물건들입니다. 무엇을 찾으시겠습니까?', list)
    if selected == nil then
        return DIALOG_RESULT.NEXT
    end

    local stored_item = me:stored_item(selected)
    local model = stored_item:model()
    local count = 1
    if model:attr(ITEM_ATTRIBUTE.BUNDLE)  then
        if stored_item:count() > 1 then
            count = me:input(npc, '얼마나 돌려드릴까요?')
            if count == nil then
                return DIALOG_RESULT.NEXT
            end
        else
            count = 1
        end

        count = tonumber(count)
        if count == nil or count <= 0 then
            return me:dialog(npc, '수량이 올바르지 않습니다.', false, true)
        end

        if stored_item:count() < count then
            return me:dialog(npc, '그만큼 맡고 있지 않습니다.', false, true)
        end

        local exists_count = 0
        local exists = me:item(model)
        if exists ~= nil then
            exists_count = exists:count()
        end
        if exists_count + count > model:capacity() then
            return me:dialog(npc, '더 이상 가질 수 없습니다.', false, true)
        end
    end
    
    if me:retrieve_item(stored_item, count) == nil then
        return me:dialog(npc, '공간이 부족합니다.', false, true)
    end

    if model:attr(ITEM_ATTRIBUTE.BUNDLE) and count > 1 then
        return me:dialog(npc, string.format('%s %d개를 돌려드렸습니다.', model:name(), count), false, true)
    else
        return me:dialog(npc, string.format('%s 돌려드렸습니다.', name_with(model:name()), count), false, true)
    end
end

function M.show_rename_weapon_menu(me, npc)
    local slots = {}
    local items = {}
    for slot, item in pairs(me:items()) do
        local model = item:model()
        if model:attr(ITEM_ATTRIBUTE.WEAPON) and model:rename_price() ~= nil then
            table.insert(slots, slot)
            items[slot] = item
        end
    end

::NPC_RENAME_WEAPON_DIALOG_001::
    local slot = me:slot(npc, '어떤 장비에 별칭을 부여하시겠어요?', slots)
    if slot == nil then
        return DIALOG_RESULT.NEXT
    end

    local weapon = items[slot]
::NPC_RENAME_WEAPON_DIALOG_002::
    local name = me:input(npc, '어떤 이름을 붙이고 싶으세요?')
    if name == nil then
        goto NPC_RENAME_WEAPON_DIALOG_001
    end

    local cp949 = CP949(name)
    if #cp949 < 4 then
        me:dialog(npc, '이름이 너무 짧습니다.', false, true)
        goto NPC_RENAME_WEAPON_DIALOG_002
    end

    if #cp949 > 32 then
        me:dialog(npc, '이름이 너무 깁니다.', false, true)
        goto NPC_RENAME_WEAPON_DIALOG_002
    end

    if not assert_korean(cp949) then
        me:dialog(npc, string.format('%s 사용할 수 없는 이름입니다.', name_with(name, '은', '는')), false, true)
        goto NPC_RENAME_WEAPON_DIALOG_002
    end

    local price = weapon:model():rename_price()
    local money = me:money()
    if price > 0 then
        if me:menu(npc, string.format('바꾸는데 %d전이 필요합니다. 정말 바꾸시겠습니까?', price), {'네', '아니오'}) == 0 then
            if price > money then
                return me:dialog(npc, '돈이 부족합니다.', false, true)
            end

            me:money(money - price)
        else
            return DIALOG_RESULT.NEXT
        end
    end

    weapon:rename(name)
    return me:dialog(npc, string.format('%s의 이름을 %s 변경했습니다.', weapon:model():name(), name_with(name, '으로', '로')), false, true)
end

function M.basic_class(me, npc, class, spells)
::NPC_BASIC_CLASS_000::
    local level = me:level()
    local class_name = class2name(class)
    if me:class() == class then
        local selected = me:menu(npc, '험난한 길을 걷는 수행자여, 무슨 일로 저를 찾으셨소?', {'마법 알아보기', '마법 배우기', '마법 지우기', '칭호 받기'})
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
                if me:dialog(npc, '내가 알려줄 마법이 없구나..', false, true) == DIALOG_RESULT.QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            table.sort(preview, function(a, b) return spells[a].level < spells[b].level end)
            selected = me:list(npc, '자네 수준이라면 이런 마법들을 알아볼 수 있겠군', preview)
            if selected == nil then
                return
            end

            local name = preview[selected+1]
            local spell = spells[name]
            if me:dialog(npc, string.format('%s %s', name_with(name, '은', '는'), spell.desc), false, true) == DIALOG_RESULT.QUIT then
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

            if me:dialog(npc, string.format('%s 배우기 위해서는 %s를 바쳐야 하네', name_with(name), table.concat(material, ', ')), false, true) == DIALOG_RESULT.QUIT then
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
                if me:dialog(npc, '내가 알려줄 마법이 없구나..', false, true) == DIALOG_RESULT.QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            table.sort(preview, function(a, b) return spells[a].level < spells[b].level end)
            selected = me:list(npc, '자네 수준이라면 이런 마법들을 배울 수 있겠군', preview)
            if selected == nil then
                return
            end

            local name = preview[selected+1]
            local spell = spells[name]
            if me:dialog(npc, string.format('%s %s', name_with(name, '은', '는'), spell.desc), false, true) == DIALOG_RESULT.QUIT then
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
            
            selected = me:list(npc, string.format('%s 배우기 위해서는 %s를 바쳐야 하네. 배우겠느냐?', name_with(name), table.concat(material, ', ')), {'예', '아니오'})
            if selected == nil then
                return
            end

            if selected == 1 then
                if me:dialog(npc, '이 모든 것은 네 탓이니, 다음에 이 곳에 올 때는 더 굳은 각오를 가지고 오도록 하거라.', false, true) == DIALOG_RESULT.QUIT then
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
                if me:dialog(npc, '필요한 것들을 구해보도록 하게. 자네라면 할 수 있겠지?', false, true) == DIALOG_RESULT.QUIT then
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
            if me:dialog(npc, '배움의 길은 끝이 없으니 더더욱 노력하는 자세를 갖도록 하거라', false, true) == DIALOG_RESULT.QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        elseif selected == 2 then
            local learned_spells = {}
            for _, spell in pairs(me:spells()) do
                table.insert(learned_spells, spell:model():name())
            end
            selected = me:list(npc, '지금 네가 지울 수 있는 마법은 다음과 같단다. 다시 한 번 심사 숙고 하고 지우도록 하여라.', learned_spells)
            if selected == nil then
                return
            end

            me:rmspell(learned_spells[selected+1])
            if me:dialog(npc, '배움의 길은 끝이 없으니 더더욱 노력하는 자세를 갖도록 하거라', false, true) == DIALOG_RESULT.QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        elseif selected == 3 then
            local title = me:input(npc, '네 정성이 갸륵하니... 그래, 무슨 칭호를 받고 싶으냐?', '받고싶은 칭호는', '입니다.', 10, true)
            if title == DIALOG_RESULT.QUIT then
                return
            end

            selected = me:list(npc, '그 칭호로 바꾸려면 금전 5000전을 바쳐야 하느니라. 네 소원을 이루겠느냐?', {'예', '아니오'})
            if selected == nil then
                return
            end

            if me:money() < 5000 then
                if me:dialog(npc, '예끼 이놈! 감히 돈도 없으면서 날 찾아와?', false, true) == DIALOG_RESULT.QUIT then
                    return
                end
                goto NPC_BASIC_CLASS_000
            end

            if me:dialog(npc, '그럼, 너의 소원을 들어주겠노라', false, true) == DIALOG_RESULT.QUIT then
                return
            end
            me:money(me:money() - 5000)
            me:title(title)
            if me:dialog(npc, '칭호를 받았으니, 자신감을 가 지고 다니게나...', false, true) == DIALOG_RESULT.QUIT then
                return
            end
            goto NPC_BASIC_CLASS_000
        end
    else
        local button = me:menu(npc, '험난한 길을 걷는 수행자여, 무슨 일로 저를 찾으셨소?', {class_name .. '직업가지기'})
        if button == 0 then
            if me:class() ~= CLASS.NONE then
                me:dialog(npc, '이미 직업이 있지 않느냐? 한번 선택한 직업은 바꿀 수 없느니라.')
                return
            end

            if level < 5 then
                me:dialog(npc, '아직 너의 정성이 부족하니 다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를 찾아오도록 하여라.')
            end
::NPC_BASIC_CLASS_001::
            button = me:dialog(npc, class_name .. '의 길을 가려면, 몇 가지 맹세를 해야하느니.', false, true)
            if button == DIALOG_RESULT.QUIT then
                return
            end

::NPC_BASIC_CLASS_002::
            local selected, button = me:list(npc, '첫째로, 하늘에서 굽어보고 계신 천제(天帝) 앞에 복종을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_BASIC_CLASS_001
            end
::NPC_BASIC_CLASS_003::
            selected, button = me:list(npc, '둘째로, 험난한 ' .. class_name .. '수련의 길에 너의 평생을 바칠 것을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_BASIC_CLASS_002
            end
::NPC_BASIC_CLASS_004::
            selected, button = me:list(npc, '셋째로, 불의를 보고 그냥 지나치지 않을 것을 맹세하겠느냐?', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            end
            if button == DIALOG_RESULT.QUIT then
                return
            end
            if button == DIALOG_RESULT.PREV then
                goto NPC_BASIC_CLASS_003
            end
::NPC_BASIC_CLASS_005::
            selected, button = me:list(npc, '훌륭하군. 그렇다면, 지금까지의 맹세를 증명하기 위해 도토리를 10개 바치거라.', {'예', '아니오'}, true)
            if selected ~= 0 then
                goto NPC_BASIC_CLASS_STOP
            elseif button == DIALOG_RESULT.QUIT then
                return
            elseif button == DIALOG_RESULT.PREV then
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
                    if me:dialog(npc, '자네는 이제 ' .. class_name .. '으로써의 끝없는 길을 가게 되었네.', false, true) == DIALOG_RESULT.QUIT then
                        return
                    end

                    if me:dialog(npc, '그 길은 멀고도 험난할 테니, 마음을 굳건히 하거라.', false, true) == DIALOG_RESULT.QUIT then
                        return
                    end
                    goto NPC_BASIC_CLASS_000
                end
            end

::NPC_BASIC_CLASS_STOP::
            me:dialog(npc, '아직 너의 정성이 부족하니 다음에 이 곳에 올 때에는 보다 큰 각오를 갖고 나를 찾아오도록 하여라.')
        end
    end
end

local PROMOTION_ITEMS = {
    [0] = { { name = '팔괘', count = 1 } },
    [1] = { { name = '수룡의비늘', count = 1 }, { name = '화룡의비늘', count = 1 } },
    [2] = { { name = '천성현의증표', count = 1 } },
    [3] = { { name = '반고의심장', count = 1 } },
}

local PROMOTION_STATS = {
    [CLASS.WARRIOR] = {
        [0] = { min_hp = 70000,  min_mp = 0 },
        [1] = { min_hp = 200000, min_mp = 0 },
        [2] = { min_hp = 600000, min_mp = 0 },
        [3] = { min_hp = 1400000, min_mp = 0 },
    },
    [CLASS.ROGUE] = {
        [0] = { min_hp = 65000,  min_mp = 0 },
        [1] = { min_hp = 220000, min_mp = 0 },
        [2] = { min_hp = 600000, min_mp = 0 },
        [3] = { min_hp = 1400000, min_mp = 0 },
    },
    [CLASS.MAGE] = {
        [0] = { min_hp = 30000,  min_mp = 20000 },
        [1] = { min_hp = 150000, min_mp = 100000 },
        [2] = { min_hp = 300000, min_mp = 500000 },
        [3] = { min_hp = 650000, min_mp = 900000 },
    },
    [CLASS.POET] = {
        [0] = { min_hp = 25000,  min_mp = 25000 },
        [1] = { min_hp = 80000,  min_mp = 70000 },
        [2] = { min_hp = 250000, min_mp = 250000 },
        [3] = { min_hp = 600000, min_mp = 750000 },
    },
}

local PROMOTION_SKILL_EXP = 50000000

local PROMOTION_SKILLS = {
    [CLASS.WARRIOR] = {
        [1] = { '백호참' },
        [2] = { '어검술', '진백호령' },
        [3] = { '초혼비무', '쇄혼비무', '포효검황' },
        [4] = { '혈겁만파', "극'백호참", '운공체식' },
    },
    [CLASS.ROGUE] = {
        [1] = { '백호검무' },
        [2] = { '이기어검' },
        [3] = { '기문방술', '무형검' },
        [4] = { '분혼경천', '파천검무', '개혈체식' },
    },
    [CLASS.MAGE] = {
        [1] = { '태양의기원' },
        [2] = { '삼매진화', '호체주술' },
        [3] = { '마기지체', '지폭지술', '노도성황', '환기' },
        [4] = { '폭류유성', '현자의기원', '만파지독' },
    },
    [CLASS.POET] = {
        [1] = { '백호의희원' },
        [2] = { '반탄공', '신령의기원' },
        [3] = { "백호의희원'첨", '신령지익', '파력무참', '환군마술' },
        [4] = { "신령의기원'첨", '봉황의기원', '귀염추혼소' },
    },
}

local function promotion_has_spell(me, spell_name)
    for _, spell in pairs(me:spells() or {}) do
        if spell:model():name() == spell_name then
            return true
        end
    end
    return false
end

function M.promotion_skills(me, npc, class)
    if me:class() ~= class then
        me:dialog(npc, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', false, true)
        return
    end
    local tier_sel = me:list(npc, '안녕하세요. 어떻게 오셨나요?', {
        '1차 승급기술',
        '2차 승급기술',
        '3차 승급기술',
    })
    if tier_sel == nil then
        return
    end
    local tier = tier_sel + 1
    if me:promotion() < tier - 1 then
        me:dialog(npc, string.format('%d차 승급을 한 뒤에 다시 오세요.', tier), false, true)
        return
    end
    local spells = PROMOTION_SKILLS[class] and PROMOTION_SKILLS[class][tier]
    if not spells or #spells == 0 then
        me:dialog(npc, '아직 준비 중입니다.', false, true)
        return
    end
    local skill_sel = me:list(npc, '안녕하세요. 어떤 기술을 배울래요?', spells)
    if skill_sel == nil then
        return
    end
    local spell_name = spells[skill_sel + 1]
    if not spell_name then
        return
    end
    if me:dialog(npc, name_with(spell_name, '을', '를') .. ' 배우기 위해선 5000만의 경험치가 필요합니다.', false, true) == DIALOG_RESULT.QUIT then
        return
    end
    local exp_now = me:exp()
    if exp_now < PROMOTION_SKILL_EXP then
        me:dialog(npc, '경험치가 모자랍니다.', false, true)
        return
    end
    if promotion_has_spell(me, spell_name) then
        me:dialog(npc, '이미 배운 마법입니다.', false, true)
        return
    end
    me:exp(exp_now - PROMOTION_SKILL_EXP)
    me:mkspell(spell_name)
    me:dialog(npc, name_with(spell_name, '을', '를') .. ' 드렸습니다.', false, true)
end

local PROMOTION_CLOTHES_ITEMS = {
    [CLASS.WARRIOR] = { [GENDER.MALE] = '검황의영혼', [GENDER.FEMALE] = '검황의심장' },
    [CLASS.ROGUE]   = { [GENDER.MALE] = '귀검의영혼', [GENDER.FEMALE] = '귀검의심장' },
    [CLASS.MAGE]    = { [GENDER.MALE] = '현인의영혼', [GENDER.FEMALE] = '현인의심장' },
    [CLASS.POET]    = { [GENDER.MALE] = '진인의영혼', [GENDER.FEMALE] = '진인의심장' },
}

function M.promotion_clothes(me, npc, class)
    if me:class() ~= class then
        me:dialog(npc, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', false, true)
        return
    end
    if me:promotion() < 3 then
        me:dialog(npc, '3차 승급을 한 뒤에 다시 오세요.', false, true)
        return
    end
    local items_by_gender = PROMOTION_CLOTHES_ITEMS[class]
    local item_name = items_by_gender and items_by_gender[me:gender()]
    if not item_name then
        me:dialog(npc, '그 옷은 아직 준비 중이에요.', false, true)
        return
    end
    if me:has_items(item_name, 1) then
        me:dialog(npc, '이미 보유 중이거나 장착 중이라 더 받을 수 없습니다.', false, true)
        return
    end

    local equipments = me:equipments()
    if equipments[EQUIPMENT_PARTS.ARMOR] and equipments[EQUIPMENT_PARTS.ARMOR]:model():name() == item_name then
        me:dialog(npc, '이미 보유 중이거나 장착 중이라 더 받을 수 없습니다.', false, true)
        return
    end
    
    if me:mkitem(item_name, 1) == nil then
        me:dialog(npc, '소지품이 가득 차서 ' .. name_with(item_name, '을', '를') .. ' 받을 수 없습니다.', false, true)
        return
    end
    me:dialog(npc, name_with(item_name, '을', '를') .. ' 드렸습니다.', false, true)
end

function M.promotion(me, npc, class)
    if me:class() ~= class then
        me:dialog(npc, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', false, true)
        return
    end
    local from_promotion = me:promotion()
    if from_promotion >= 3 then
        me:dialog(npc, '당신은 더 이상 제가 수련을 도와드리지 않아도 될 만큼 성장하셨군요.', false, true)
        return
    end
    local stats = PROMOTION_STATS[class] and PROMOTION_STATS[class][from_promotion]
    local items = PROMOTION_ITEMS[from_promotion]
    if not stats or not items then
        me:dialog(npc, string.format('아직 %d차 이상 승급은 준비 중입니다.', from_promotion + 2), false, true)
        return
    end
    if me:level() < 99 then
        me:dialog(npc, '좀더 수련을 쌓고오세요.', false, true)
        return
    end
    if me:base_hp() < stats.min_hp or me:base_mp() < stats.min_mp then
        me:dialog(npc, '좀더 수련을 쌓고오세요.', false, true)
        return
    end
    if me:dialog(npc, string.format('어서 오세요. 소녀는 %s분들의 수호천녀 %s입니다.', class2name(class, 0), npc:model():name()), false, true) == DIALOG_RESULT.QUIT then
        return
    end
    local next_name = class2name(class, from_promotion + 1)
    if not next_name then
        return
    end
    local selected, button = me:list(npc, string.format('이토록 강해지시다니, 정말 대단하십니다.\n당신이 지금껏 걸어온 고된 수련의 길에 경의를 표합니다.\n\n지금 %s의 칭호를 받으시겠습니까?', next_name), {'네, 모든 준비가 끝났습니다.', '나중에 다시 오지요.'})
    if selected ~= 0 then
        return
    end
    for _, it in ipairs(items) do
        if not me:rmitem(it.name, it.count, ITEM_DELETE_TYPE.GIVE) then
            me:dialog(npc, name_with(it.name, '이', '가') .. ' 없습니다.', false, true)
            return
        end
    end
    me:promotion(from_promotion + 1)
    broadcast(string.format('(( [%s]님이 %s 승급하였습니다. 축하합니다! ))', me:name(), name_with(next_name, '으로', '로')), MESSAGE_TYPE.WORLD, BROADCAST_TYPE.WORLD)
    me:dialog(npc, '승급을 마쳤습니다. 더 높은 경지에 도전하시길 바랍니다.', false, true)
end

function M.revive_oath(me, npc)
    if me:state() ~= STATE.GHOST then
        return me:dialog(npc, '사망 상태가 아닐 때 나오는 메시지')
    end

    if me:menu(npc, '생명의 소중함을 그렇게 일러왔거늘... 앞으로돋 사소한 일에 목숨을걸지 않으리라고 내가 어떻게 믿을 수 있겠느냐? 또 생명을 잃고 나를 찾아오지 않겠다고 맹세할 수 있겠느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '싸움은 싸움을 부르고, 피는 반드시 피를 보게 되느니라. 이번의 죽음도 네 책임이라는 것을 진심으로 느끼고 반성하고 있느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '그렇다면 잃은 물건과 경험치도 다 네 욕심에서 비롯되었음을 인정하겠느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    if me:menu(npc, '네가 새로 생명을 얻게 되더라도 절대로 무고한 생명을 해치지 않을 것을 맹세하느냐?', {'예', '아니오'}) ~= 0 then
        return me:dialog(npc, '부활 대답 거부 메시지')
    end

    me:dialog(npc, '너의 각오를 믿고 새로운 생명을 내리노니 나에게 한 맹세를 잊지 말고 하루하루를 신께 감사하는 마음으로 살아가도록 하여라.')
    me:state(STATE.NORMAL)
    me:hp(50)
end

function M.boss_challenge_gate(me, npc)
    local button = me:dialog(npc, '내게 도전하고 싶거든, 먼저 병사들을 모두 물리치고 오너라.', false, false)
    if button == DIALOG_RESULT.QUIT then
        return
    end
end

function M.yut_game(me, npc)
    local YUT_COST = 10

    local function yut_result()
        local r = math.random(1, 16)
        if r == 1 then
            return '도', false
        elseif r >= 2 and r <= 4 then
            return '개', false
        elseif r >= 5 and r <= 10 then
            return '걸', false
        elseif r >= 11 and r <= 14 then
            return '윷', true
        elseif r == 15 then
            return '모', true
        else
            return '빽도', false
        end
    end

    if me:money() < YUT_COST then
        npc:chat(string.format('%s: [%s]님. 금전이 부족하시네요. 10전을 가져오세요.', npc:model():name(), me:name()))
        return
    end

    me:money(me:money() - YUT_COST)
    local pae, again = yut_result()
    local plus = again and ' 한 번 더 던지세요.' or ''
    npc:chat(string.format('%s: [%s]님. %s 나왔습니다.%s', npc:model():name(), me:name(), pae, plus))
end

function M.nakrang_intro(me, npc)
    local quest = require('lib.quest')
    local q = me:quest(quest.QUEST_NAKRANG_INTRO)
    local already_got = (q ~= nil and q:completed())
    if already_got then
        me:dialog(npc, '두루마리를 열어보기\n위해서는 오른쪽의\n\'소지품(단축키i)\'을\n클릭하신 다음 제가 드린\n두루마리를 더블클릭하시면 됩니다.', false, false)
        return
    end

    local btn
    ::nakrang_intro_1::
    btn = me:dialog(npc, me:name() .. '님, 안녕하세요?\n바람의나라에 오신 것을\n환영합니다!!!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    ::nakrang_intro_2::
    btn = me:dialog(npc, '드넓은 바람의 세계로 가시기\n전에 \'두루마리\'를 하나\n드릴테니, 시작하시기 전에\n꼭!! 열어 보세요..', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto nakrang_intro_1
    end
    ::nakrang_intro_3::
    btn = me:dialog(npc, '열어보기 위해서는 오른쪽의\n\'소지품(단축키i)\'을\n클릭하신 다음 제가 드린\n두루마리를 더블클릭하시면\n됩니다.', true, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto nakrang_intro_2
    end
    ::nakrang_intro_4::
    btn = me:dialog(npc, '자. 그럼, 머나먼 모험의\n길을 떠나 보시기 바랍니다~\n제가 드리는 두루마리 꼭!!!\n열어보세요!!!', false, true)
    if btn == DIALOG_RESULT.QUIT then
        return
    end
    if btn == DIALOG_RESULT.PREV then
        goto nakrang_intro_3
    end

    if q == nil then
        q = me:start_quest(quest.QUEST_NAKRANG_INTRO)
        if q == nil then
            me:dialog(npc, '두루마리를 받을 수 없습니다.')
            return
        end
    end
    q:complete()
    local item = me:mkitem('낙랑의두루마리1', 1)
    me:dialog(item, '<낙랑의두루마리1>을 얻다!!!', false, false)
end

return M
