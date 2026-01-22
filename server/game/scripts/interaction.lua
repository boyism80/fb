function string_split(self, delimiter)
    local result = { }
    local from  = 1
    local delim_from, delim_to = string.find( self, delimiter, from  )
    while delim_from do
        table.insert( result, string.sub( self, from , delim_from-1 ) )
        from  = delim_to + 1
        delim_from, delim_to = string.find( self, delimiter, from  )
    end
    table.insert( result, string.sub( self, from  ) )
    return result
end

function is_miss(me, you)
    if debug() then
        return false
    else
        return math.random() > 0.8
    end
end

function is_critical(me, you)
    if debug() then
        return true
    else
        return math.random() > 0.8
    end
end

function is_back_attack(me, you)
    local direction1 = me:direction()
    local direction2 = you:direction()

    if direction1 ~= direction2 then
        return false
    end

    local x1, y1 = me:position()
    local x2, y2 = you:position()
    if direction1 == DIRECTION.LEFT then
        return x1 > x2
    elseif direction1 == DIREECTION_TOP then
        return y1 > y2
    elseif direction1 == DIRECTION.RIGHT then
        return x1 < x2
    else
        return y1 < y2
    end
end

function damage(me, you, rate, sound)
    if rate == nil then
        rate = 1.0
    end

    local size = MOB_SIZE.SMALL
    if you:is(OBJECT_TYPE.MOB) then
        local model = you:model()
        size = model:size()
    end

    local damage = me:normal_attack_damage(size)
    local damage_rate = me:damage_rate() / 1000.0
    if is_back_attack(me, you) then
        damage_rate = damage_rate * 2
    end

    local critical = is_critical()
    if critical then
        damage_rate = damage_rate * 2
    end

    if me:isbuff('투명') then
        damage_rate = damage_rate * 8
    end

    damage_rate = damage_rate / (you:damage_derate() / 1000.0)
    if sound ~= nil then
        you:sound(sound)
    end
    
    local damage = math.floor(damage * damage_rate * rate)
    you:damage(damage, me, critical)
end

function on_attack(me, additional_attack)
    local map = me:map()
    if map == nil then
        return 0
    end

    local option = map:model():option()
    local pk = (option & MAP_OPTION.ENABLE_PK) == MAP_OPTION.ENABLE_PK
    local enemy_type = OBJECT_TYPE.LIFE
    if not pk then
        enemy_type = OBJECT_TYPE.MOB
    end

    if additional_attack == nil then
        additional_attack = false
    end

    if not additional_attack then
        me:action(ACTION.ATTACK, DURATION.ATTACK)
    end

    local weapon = nil
    local is_bow = false
    if me:is(OBJECT_TYPE.CHARACTER) and not additional_attack then
        weapon = me:weapon()
        if weapon ~= nil then
            local model = weapon:model()
            is_bow = model:type() == WEAPON_TYPE.BOW

            local sound = model:sound()
            if sound == 0 then
                if not is_bow then
                    sound = SOUND.SWING
                end
            end
            me:sound(sound)
        end
    end

    local count = 0
    local x, y = me:position()
    local direction = me:direction()
    if is_bow then
        local range = 14
        local target = nil
        local width = map:width()
        local height = map:height()
        for i = 1, range do
            local obj_x = x
            local obj_y = y
            if direction == DIRECTION.LEFT then
                obj_x = obj_x - i
            elseif direction == DIRECTION.RIGHT then
                obj_x = obj_x + i
            elseif direction == DIRECTION.TOP then
                obj_y = obj_y - i
            else
                obj_y = obj_y + i
            end

            if obj_x < 0 or obj_x > width or obj_y < 0 or obj_y > height then
                break
            end

            target = map:at(obj_x, obj_y, enemy_type)
            if target ~= nil then
                break
            end
        end

        if target ~= nil then
            damage(me, target, nil, 701)
            count = count + 1
        end
    else
        local front = nil
        if me:is(OBJECT_TYPE.MOB) then
            front = me:target()
            if front ~= nil then
                local x_front, y_front = front:position()
                local x_diff = math.abs(x_front - x)
                local y_diff = math.abs(y_front - y)
                local is_near = (x_diff == 1 and y_diff == 0) or (y_diff == 1 and x_diff == 0)
                if not is_near then
                    front = nil
                end
            end
        end
        if front == nil then
            front = me:front(enemy_type)
        end
        local damaged_sound = nil
        if weapon ~= nil then
            damaged_sound = SOUND.DAMAGE
        end
        if front ~= nil and not is_miss(me, front) then
            damage(me, front, nil, damaged_sound)
            count = count + 1
        end

        if me:isbuff('측면공격') then
            local points = {}
            if direction == DIRECTION.LEFT or direction == DIRECTION.RIGHT then
                table.insert(points, {x, y-1})
                table.insert(points, {x, y+1})
            else
                table.insert(points, {x-1, y})
                table.insert(points, {x+1, y})
            end

            local nears = me:nears(enemy_type, points, false)
            for _, obj in pairs(nears) do
                damage(me, obj, 0.4, damaged_sound)
                count = count + 1
            end
        end

        if me:isbuff('후면공격') then
            local points = {}
            if direction == DIRECTION.LEFT then
                table.insert(points, {x+1, y})
            elseif direction == DIRECTION.TOP then
                table.insert(points, {x, y+1})
            elseif direction == DIRECTION.RIGHT then
                table.insert(points, {x-1, y})
            else
                table.insert(points, {x, y-1})
            end

            local nears = me:nears(enemy_type, points, false)
            for _, obj in pairs(nears) do
                damage(me, obj, 0.5, damaged_sound)
                count = count + 1
            end
        end
    end

    if me:isbuff('투명') and count > 0 then
        me:unbuff('투명')
    end

    if me:isbuff('분신') and not additional_attack then
        count = count + on_attack(me, true)
    end

    any_action(me)
    return count
end

function on_equipment_active(me, parts, equipment)
    any_action(me)
end

function on_equipment_inactive(me, parts, equipment)
    if me:weapon_damage() > 0 then
        me:message('무기의 푸른빛이 사라집니다.')
        me:weapon_damage(0)
    end

    any_action(me)
end

function on_loot(me)
    for _, buff_name in pairs(relative_buff_name('투명')) do
        if me:isbuff(buff_name) then
            me:state(STATE.NORMAL)
            me:unbuff(buff_name)
        end
    end

    any_action(me)
end

function on_door(me)
    local map = me:map()
    local door = map:door(me)
    if door == nil then
        return
    end

    local key = me:item('파란열쇠')
    local locked = door:locked()
    if door:locked() and key == nil then
        me:message('문이 잠겨있습니다.')
        return
    end

    local opened = door:toggle()
    if opened then
        me:message('문을 열었습니다.')
    else
        me:message('문을 닫았습니다.')
    end

    any_action(me)
end

function on_move(me)
    any_action(me)
end

function on_direction(me)
    any_action(me)
end

function any_action(me)
    if me:isbuff('운기') then
        me:unbuff('운기')
    end
end

function on_chat(me, message, shout)
    if string.sub(message, 1, 1) == '/' then
        message = string.sub(message, 2, string.len(message))
        args = string_split(message, ' ')

        local cmd = args[1]
        if command_funcs[cmd] == nil then
            return false
        end
        
        local cmd_data = command_funcs[cmd]
        local cmd_func = nil
        local required_privilege = ROLE.USER
        
        if type(cmd_data) == 'table' then
            cmd_func = cmd_data['command']
            required_privilege = cmd_data['privilege'] or ROLE.USER
        else
            cmd_func = cmd_data
        end
        
        if me:role() < required_privilege then
            me:message("권한이 부족합니다.")
            return true
        end
        
        table.remove(args, 1)
        return cmd_func(me, args)
    end
    
    on_npc_chat(me, message, shout)
    return false
end

function on_npc_chat(me, message, shout)
    local map = me:map()
    if map == nil then
        return false
    end
    
    local npcs = {}
    if shout then
        npcs = map:objects(OBJECT_TYPE.NPC)
    else
        local x, y = me:position()
        npcs = map:nears({x, y}, OBJECT_TYPE.NPC)
    end
    
    if #npcs == 0 then
        return false
    end
    
    local regex_handlers = {
        { pattern = CONST.REGEX.BUY, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            local name = params.name
            local count = 1
            if params.count ~= nil then
                count = tonumber(params.count)
            end
            return npc_sell_item(me, npc, name, count)
        end },
        { pattern = CONST.REGEX.SELL, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            local name = params.name
            local count = nil
            if params.count ~= nil then
                count = tonumber(params.count)
            elseif params.all ~= nil then
                count = nil
            end
            return npc_buy_item(me, npc, name, count)
        end },
        { pattern = CONST.REGEX.REPAIR, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REPAIR) == NPC_INTERACTION.REPAIR
        end, func = function(npc, params)
            if params.all ~= nil then
                return npc_repair(me, npc, nil)
            elseif params.name ~= nil then
                return npc_repair(me, npc, params.name)
            end
            return false
        end },
        { pattern = CONST.REGEX.DEPOSIT_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            local money = tonumber(params.money)
            if money == nil then
                return false
            end
            return npc_deposit_money(me, npc, money)
        end },
        { pattern = CONST.REGEX.WITHDRAW_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            local money = tonumber(params.money)
            if money == nil then
                return false
            end
            return npc_withdraw_money(me, npc, money)
        end },
        { pattern = CONST.REGEX.STORE_ITEM, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            local count = 1
            if params.count ~= nil then
                count = tonumber(params.count)
            end
            return npc_store_item(me, npc, name, count)
        end },
        { pattern = CONST.REGEX.RETRIEVE_ITEM, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            local count = 1
            if params.count ~= nil then
                count = tonumber(params.count)
            end
            return npc_retrieve_item(me, npc, name, count)
        end },
        { pattern = CONST.REGEX.SELL_LIST, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            return npc_sell_item_list(me, npc)
        end },
        { pattern = CONST.REGEX.BUY_LIST, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            return npc_buy_item_list(me, npc)
        end },
        { pattern = CONST.REGEX.SELL_PRICE, condition = function(npc)
            local model = npc:model()
            local sell = model:sell()
            return #sell > 0
        end, func = function(npc, params)
            local name = params.name
            return npc_sell_item_price(me, npc, name)
        end },
        { pattern = CONST.REGEX.BUY_PRICE, condition = function(npc)
            local model = npc:model()
            local buy = model:buy()
            return buy ~= nil
        end, func = function(npc, params)
            local name = params.name
            return npc_buy_item_price(me, npc, name)
        end },
        { pattern = CONST.REGEX.DEPOSITED_MONEY, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.DEPOSIT_MONEY) == NPC_INTERACTION.DEPOSIT_MONEY
        end, func = function(npc, params)
            return npc_deposited_money(me, npc)
        end },
        { pattern = CONST.REGEX.RENAME_WEAPON, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.RENAME) == NPC_INTERACTION.RENAME
        end, func = function(npc, params)
            local from = params.from
            local to = params.to
            return npc_rename_weapon(me, npc, from, to)
        end },
        { pattern = CONST.REGEX.HOLD_ITEM_LIST, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            return npc_store_item_list(me, npc)
        end },
        { pattern = CONST.REGEX.HOLD_ITEM_COUNT, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.STORE_ITEM) == NPC_INTERACTION.STORE_ITEM
        end, func = function(npc, params)
            local name = params.name
            return npc_store_item_count(me, npc, name)
        end },
        { pattern = CONST.REGEX.REVIVE, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end, func = function(npc, params)
            local discourteous = params.discourteous ~= nil
            return npc_revive(me, npc, discourteous)
        end },
        { pattern = CONST.REGEX.APPRECIATE, condition = function(npc)
            local model = npc:model()
            local interaction = model:interaction()
            return (interaction & NPC_INTERACTION.REVIVE) == NPC_INTERACTION.REVIVE
        end, func = function(npc, params)
            return npc_appreciate(me, npc)
        end },
    }

    for _, handler in ipairs(regex_handlers) do
        local params = regex(handler.pattern, message)
        if params ~= nil then
            for _, npc in ipairs(npcs) do
                if handler.condition ~= nil then
                    if not handler.condition(npc) then
                        goto continue
                    end
                end
                if handler.func(npc, params) then
                    return true
                end
                ::continue::
            end
        end
    end
    
    return false
end

function on_login(me)
    if me:birthday() ~= nil then
        return
    end

    local npc = name2npc('낙랑')
    local button = nil
::BIRTHDAY_DIALOG_1::
    button = me:dialog(npc, '대단히 중요하니 끝까지 읽어 주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.', false, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

::BIRTHDAY_DIALOG_2::
    button = me:dialog(npc, '대충 2차 비밀번호 설정하라고 강경하게 말하는 내용', true, true)
    if button == DIALOG_RESULT.QUIT then
        return
    end

    if button == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_1
    end

::BIRTHDAY_DIALOG_3::
    local birthday = me:input(npc, '2차 비밀번호 설정 뭘로 할래요?', '내 생년월일은,', '입니다.', 6, true)
    if birthday == DIALOG_RESULT.QUIT then
        return
    end

    if birthday == DIALOG_RESULT.PREV then
        goto BIRTHDAY_DIALOG_2
    end

    if birthday == '' then
        button = me:dialog(npc, '제대로 입력하세요.', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end
        goto BIRTHDAY_DIALOG_3
    end

    if #birthday ~= 6 then
        button = me:dialog(npc, '생년월일이 너무 짧습니다.', false, true)
        if button == DIALOG_RESULT.QUIT then
            return
        end

        goto BIRTHDAY_DIALOG_3
    end

    local answer = me:list(npc, string.format('당신의 생년월일이 %s가 맞습니까?', birthday), {'예', '아니오'})
    if answer == nil then
        return
    end

    if answer == 1 then
        goto BIRTHDAY_DIALOG_3
    end

    me:birthday(birthday)
end