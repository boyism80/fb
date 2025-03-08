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
    if direction1 == DIRECTION_LEFT then
        return x1 > x2
    elseif direction1 == DIREECTION_TOP then
        return y1 > y2
    elseif direction1 == DIRECTION_RIGHT then
        return x1 < x2
    else
        return y1 < y2
    end
end

function damage(me, you, rate, sound)
    if rate == nil then
        rate = 1.0
    end

    local size = MOB_SIZE_SMALL
    if you:is(OBJECT_TYPE_MOB) then
        local model = you:model()
        size = model:size()
    end

    local damage = 0
    if me:is(OBJECT_TYPE_CHARACTER) then
        local weapon = me:weapon()
        if weapon == nil then
            damage = math.random(1, 5)
        elseif size == MOB_SIZE_SMALL then
            local min, max = weapon:model():damage_small()
            damage = math.random(min, max) + me:weapon_damage()
        else
            local min, max = weapon:model():damage_large()
            damage = math.random(min, max) + me:weapon_damage()
        end
    else
        local min, max = me:model():damage()
        damage = math.random(min, max)
    end

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
    local pk = (option & MAP_OPTION_ENABLE_PK) == MAP_OPTION_ENABLE_PK
    local enemy_type = OBJECT_TYPE_LIFE
    if not pk then
        enemy_type = OBJECT_TYPE_MOB
    end

    if additional_attack == nil then
        additional_attack = false
    end

    if not additional_attack then
        me:action(ACTION_ATTACK, DURATION_ATTACK)
    end

    local weapon = nil
    local is_bow = false
    if me:is(OBJECT_TYPE_CHARACTER) and not additional_attack then
        weapon = me:weapon()
        if weapon ~= nil then
            local model = weapon:model()
            is_bow = model:type() == WEAPON_TYPE_BOW

            local sound = model:sound()
            if sound == 0 then
                if not is_bow then
                    sound = SOUND_SWING
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
            if direction == DIRECTION_LEFT then
                obj_x = obj_x - i
            elseif direction == DIRECTION_RIGHT then
                obj_x = obj_x + i
            elseif direction == DIRECTION_TOP then
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
        if me:is(OBJECT_TYPE_MOB) then
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
            damaged_sound = SOUND_DAMAGE
        end
        if front ~= nil and not is_miss(me, front) then
            damage(me, front, nil, damaged_sound)
            count = count + 1
        end

        if me:isbuff('측면공격') then
            local points = {}
            if direction == DIRECTION_LEFT or direction == DIRECTION_RIGHT then
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
            if direction == DIRECTION_LEFT then
                table.insert(points, {x+1, y})
            elseif direction == DIRECTION_TOP then
                table.insert(points, {x, y+1})
            elseif direction == DIRECTION_RIGHT then
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

function on_pickup(me)
    for _, buff_name in pairs(relative_buff_name('투명')) do
        if me:isbuff(buff_name) then
            me:state(STATE_NORMAL)
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

function on_chat(me, message)
    if string.sub(message, 1, 1) ~= '/' then
        return false
    end

    message = string.sub(message, 2, string.len(message))
    args = message:split(' ')

    local cmd = args[1]
    if command_funcs[cmd] == nil then
        return false
    end
    
    table.remove(args, 1)
    return command_funcs[cmd](me, args)
end

function on_login(me)
    if me:birthday() ~= nil then
        return
    end

    local npc = name2npc('낙랑')
    local button = nil
::BIRTHDAY_DIALOG_1::
    button = npc:dialog(me, '대단히 중요하니 끝까지 읽어 주세요! 빈번히 발생하는 아이디 해킹을 미연에 방지하기 위해 또 하나의 2차 비밀번호를 정해야 합니다.', false, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end

::BIRTHDAY_DIALOG_2::
    button = npc:dialog(me, '대충 2차 비밀번호 설정하라고 강경하게 말하는 내용', true, true)
    if button == DIALOG_RESULT_QUIT then
        return
    end

    if button == DIALOG_RESULT_PREV then
        goto BIRTHDAY_DIALOG_1
    end

::BIRTHDAY_DIALOG_3::
    local birthday = npc:input(me, '2차 비밀번호 설정 뭘로 할래요?', '내 생년월일은,', '입니다.', 6, true)
    if birthday == DIALOG_RESULT_QUIT then
        return
    end

    if birthday == DIALOG_RESULT_PREV then
        goto BIRTHDAY_DIALOG_2
    end

    if birthday == '' then
        button = npc:dialog(me, '제대로 입력하세요.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end
        goto BIRTHDAY_DIALOG_3
    end

    if #birthday ~= 6 then
        button = npc:dialog(me, '생년월일이 너무 짧습니다.', false, true)
        if button == DIALOG_RESULT_QUIT then
            return
        end

        goto BIRTHDAY_DIALOG_3
    end

    local answer = npc:list(me, string.format('당신의 생년월일이 %s가 맞습니까?', birthday), {'예', '아니오'})
    if answer == nil then
        return
    end

    if answer == 1 then
        goto BIRTHDAY_DIALOG_3
    end

    me:birthday(birthday)
end