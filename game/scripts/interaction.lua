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

function damage(me, you, rate)
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

    local damage_rate = 1.0 + me:damage_rate() / 1000.0
    if me:isbuff('투명') then
        damage_rate = damage_rate * 8
    end
    
    if is_back_attack(me, you) then
        damage_rate = damage_rate * 2
    end

    local critical = is_critical()
    if critical then
        damage_rate = damage_rate * 2
    end

    damage_rate = damage_rate / (you:damage_derate() / 1000.0)
    if me:is(OBJECT_TYPE_CHARACTER) then
        you:sound(SOUND_DAMAGE)
    end
    you:damage(math.floor(damage * damage_rate * rate), me, critical)
end

function on_attack(me)
    me:action(ACTION_ATTACK, DURATION_ATTACK)

    if me:is(OBJECT_TYPE_CHARACTER) then
        local weapon = me:weapon()
        if weapon ~= nil then
            local model = weapon:model()
            local sound = model:sound()
            me:sound(sound)
        end
    end

    local count = 0
    local front = me:front(OBJECT_TYPE_LIFE)
    if front ~= nil and not is_miss(me, front) then
        damage(me, front)
        count = count + 1
    end

    local nears = me:nears(OBJECT_TYPE_LIFE, 1, 1)
    local x, y = me:position()
    local direction = me:direction()
    if me:isbuff('측면공격') then
        local points = {}
        if direction == DIRECTION_LEFT or direction == DIRECTION_RIGHT then
            table.insert(points, {x, y-1})
            table.insert(points, {x, y+1})
        else
            table.insert(points, {x-1, y})
            table.insert(points, {x+1, y+1})
        end

        for _, obj in pairs(nears) do
            local obj_x, obj_y = obj:position()
            for _, point in pairs(points) do
                local point_x, point_y = table.unpack(point)
                if obj_x == point_x and obj_y == point_y and not is_miss(me, obj) then
                    damage(me, obj, 0.4)
                    break
                end
            end
        end
    end

    if me:isbuff('후면공격') then
        local point = nil
        if direction == DIRECTION_LEFT then
            point = {x+1, y}
        elseif direction == DIRECTION_TOP then
            point = {x, y+1}
        elseif direction == DIRECTION_RIGHT then
            point = {x-1, y}
        else
            point = {x, y-1}
        end

        for _, obj in pairs(nears) do
            local obj_x, obj_y = obj:position()
            local point_x, point_y = table.unpack(point)
            if obj_x == point_x and obj_y == point_y and not is_miss(me, obj) then
                damage(me, obj, 0.5)
                break
            end
        end
    end

    if me:isbuff('투명') and count > 0 then
        me:unbuff('투명')
    end
end

function on_equipment_active(me, parts, equipment)
    
end

function on_equipment_inactive(me, parts, equipment)
    if me:weapon_damage() > 0 then
        me:message('무기의 푸른빛이 사라집니다.')
        me:weapon_damage(0)
    end
end

function on_pickup(ch)
    if ch:state() == 0x05 then
        ch:state(0x00)
        ch:unbuff('투명')
    end
end

function on_door(ch)
    local map = ch:map()
    local door = map:door(ch)
    if door == nil then
        return
    end

    local key = ch:item('파란열쇠')
    local locked = door:locked()
    if door:locked() and key == nil then
        ch:message('문이 잠겨있습니다.')
        return
    end

    local opened = door:toggle()
    if opened then
        ch:message('문을 열었습니다.')
    else
        ch:message('문을 닫았습니다.')
    end
end