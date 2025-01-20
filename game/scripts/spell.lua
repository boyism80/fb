function relative_buff_name(buff_name)
    local values = {
        {'혼마술', '저주', '귀염추혼소'},
        {'무장', '자동무장'},
        {'보호', '자동보호'}
    }

    for _, names in pairs(values) do
        for _, name in pairs(names) do
            if buff_name == name then
                return names
            end
        end
    end

    return {buff_name}
end

function near(me, type)
    local result = {}
    local map = me:map()
    if map == nil then
        return result
    end

    local x, y = me:position()
    for _, obj in pairs(map:objects()) do
        if not obj:is(type) then
            goto SKIP
        end

        local obj_x, obj_y = obj:position()
        if (obj_x == x+1 or obj_x == x-1) and (obj_y == y) then
            table.insert(result, obj)
        elseif (obj_y == y+1 or obj_y == y-1) and (obj_x == x) then
            table.insert(result, obj)
        else
            
        end
        ::SKIP::
    end

    return result
end

function spell_cast(me, you, spell, mp, sound, effect, no_assert)
    if no_assert == nil then
        no_assert = false
    end

    if not no_assert then
        local error = me:assert_state(STATE_GHOST, STATE_RIDING)
        if error ~= nil then
            me:message(error)
            return
        end
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    return true
end

function buff_cast(me, you, spell, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if you:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        me:message('이미 걸려있습니다.')
        return false
    end

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    return true
end

function debuff_cast(me, you, spell, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if you:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        me:message('이미 걸려있습니다.')
        return false
    end

    you:effect(effect)
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 걸었습니다.', me:name(), name_with(spell:name())))
    end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    return true
end


function attack_cast(me, you, spell, hp, mp, damage, message, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if you == nil then
        return false
    end

    if type(you) == 'userdata' then
        you = {you}
    end
    if #you == 0 then
        return false
    end

    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    me:action(ACTION_ATTACK, DURATION_ATTACK, 1)

    local damaged = false
    local rate = me:skill_damage_rate() / 1000.0
    for _, obj in pairs(you) do
        obj:effect(effect)
        obj:sound(sound)
        if obj:is(OBJECT_TYPE_LIFE) then
            obj:damage(math.floor(damage * rate), me)
            damaged = true
        end
    end

    if damaged then
        me:hp(math.max(10, me:hp() - hp))
    end
    me:chat(message, CHAT_TYPE_BLUE, false)
    return true
end

function spell_damage(me, you, spell, damage, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    you:effect(effect)
    you:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    if you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
    end

    local rate = me:skill_damage_rate() / 1000.0
    you:damage(math.floor(damage*rate), me)
    return true
end

function spell_damage_near(me, spell, damage, mp, sound, effect)
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    me:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)

    local rate = me:skill_damage_rate() / 1000.0
    for _, you in pairs(near(me, OBJECT_TYPE_LIFE)) do
        you:effect(effect)
        if you:is(OBJECT_TYPE_CHARACTER) then
            you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        you:damage(math.floor(damage*rate), me)
    end

    return true
end

function spell_damage_area(me, you, spell, damage, mp, sound, effect_me, effect_you)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if type(you) == 'userdata' then
        you = {you}
    end

    local rate = me:skill_damage_rate() / 1000.0
    for _, obj in pairs(you) do
        if obj:is(OBJECT_TYPE_CHARACTER) then
            obj:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        if effect_you ~= nil then
            obj:effect(effect_you)
        end
        obj:damage(math.floor(damage*rate), me)
    end
    me:sound(sound)
    if effect_me ~= nil then
        me:effect(effect_me)
    end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    return true
end

function spell_heal(me, you, spell, hp, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end

    if not spell_cast(me, you, spell, mp, sound, effect) then
        return
    end

    you:heal(hp)
    return true
end

function spell_heal_near(me, you, spell, hp, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return
    end
    
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return
    end
    me:mp_down(mp)

    local targets = near(you, OBJECT_TYPE_CHARACTER)
    table.insert(targets, you)

    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(targets) do
        ch:heal(hp)
        ch:effect(effect)
        ch:sound(sound)
        if me ~= ch then
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
    return true
end

function spell_heal_group(me, spell, hp, mp, sound, effect)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    local map = me:map()
    if map == nil then
        return false
    end

    local group = me:group()
    if group == nil then
        me:message('가입된 그룹이 없습니다.')
        return false
    end

    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return false
    end
    me:mp_down(mp)

    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))

    for _, ch in pairs(group:nears(map, {me:position()})) do
        if me ~= ch then
            ch:heal(hp)
            ch:effect(effect)
            ch:sound(sound)
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
    return true
end

function spell_disguise_look(me, mobs, name)
    for k, v in pairs(mobs) do
        if k == name then
            return v
        end
    end

    me:message('변신 가능한 동물')
    for k, v in pairs(mobs) do
        me:message(k)
    end
    return nil
end

function spell_disguise(me, mobs, name, spell, mp, sound, effect, buff_time)
    local error = me:assert_state(STATE_GHOST, STATE_RIDING)
    if error ~= nil then
        me:message(error)
        return false
    end

    local look = spell_disguise_look(me, mobs, name)
    if look == nil then
        return
    end

    if buff_cast(me, me, spell, mp, sound, effect) then
        me:disguise(look)
        me:buff(spell, buff_time)
    end
end

function unbuff(me, spell)
    if me:is(OBJECT_TYPE_CHARACTER) then
        me:message(string.format('%s 해제', spell:name()))
    end
end

function front_obj(x, y, direction, step, objects, type)
    if type == nil then
        type = 0xFF & ~OBJECT_TYPE_ITEM
    end

    if direction == DIRECTION_LEFT then
        x = x-step
    elseif direction == DIRECTION_RIGHT then
        x = x+step
    elseif direction == DIRECTION_TOP then
        y = y-step
    else
        y = y+step
    end

    for _, obj in pairs(objects) do
        local obj_x, obj_y =  obj:position()
        if x == obj_x and y == obj_y and obj:is(type) then
            return obj
        end
    end

    return nil
end

function failed_attack_spell(me)
    local message = '허공난무 흐미 실패닷'
    me:chat(message, CHAT_TYPE_BLUE)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE_SHOUT, BROADCAST_TYPE_WORLD)
end