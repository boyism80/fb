function boolean_random(percent)
    return math.random() < percent
end

function CREATURE_SPELL(creature, index)
    if creature == CREATURE_PHOENIX then
        if index == 1 then
            return '화염주'
        elseif index == 2 then
            return '화영열주'
        elseif index == 3 then
            return '화열참주'
        elseif index == 4 then
            return '진화열참주'
        elseif index == 5 then
            return '극진화열참주'
        else
            return nil
        end

    elseif creature == CREATURE_TIGER then
        if index == 1 then
            return '백열주'
        elseif index == 2 then
            return '백령주'
        elseif index == 3 then
            return '백열참주'
        elseif index == 4 then
            return '진백열참주'
        elseif index == 5 then
            return '극진백열참주'
        else
            return nil
        end

    elseif creature == CREATURE_TURTLE then
        if index == 1 then
            return '자무주'
        elseif index == 2 then
            return '자영무주'
        elseif index == 3 then
            return '자천무주'
        elseif index == 4 then
            return '진자천무주'
        elseif index == 5 then
            return '극진자천무주'
        else
            return nil
        end
    elseif creature == CREATURE_DRAGON then
        if index == 1 then
            return '뢰진주'
        elseif index == 2 then
            return '뢰격주'
        elseif index == 3 then
            return '뢰격참주'
        elseif index == 4 then
            return '진뢰격참주'
        elseif index == 5 then
            return '극진뢰격참주'
        else
            return nil
        end
    else
        return nil
    end
end

function CREATURE_AREA_SPELL(creature, index)
    if creature == CREATURE_PHOENIX then
        if index == 1 then
            return "화염주'첨"
        elseif index == 2 then
            return "화영열주'첨"
        elseif index == 3 then
            return "화열참주'첨"
        elseif index == 4 then
            return "진화열참주'첨"
        elseif index == 5 then
            return "극진화열참주'첨"
        else
            return nil
        end

    elseif creature == CREATURE_TIGER then
        if index == 1 then
            return "백열주'첨"
        elseif index == 2 then
            return "백령주'첨"
        elseif index == 3 then
            return "백열참주'첨"
        elseif index == 4 then
            return "진백열참주'첨"
        elseif index == 5 then
            return "극진백열참주'첨"
        else
            return nil
        end

    elseif creature == CREATURE_TURTLE then
        if index == 1 then
            return "자무주'첨"
        elseif index == 2 then
            return "자영무주'첨"
        elseif index == 3 then
            return "자천무주'첨"
        elseif index == 4 then
            return "진자천무주'첨"
        elseif index == 5 then
            return "극진자천무주'첨"
        else
            return nil
        end
    elseif creature == CREATURE_DRAGON then
        if index == 1 then
            return "뢰진주'첨"
        elseif index == 2 then
            return "뢰격주'첨"
        elseif index == 3 then
            return "뢰격참주'첨"
        elseif index == 4 then
            return "진뢰격참주'첨"
        elseif index == 5 then
            return "극진뢰격참주'첨"
        else
            return nil
        end
    else
        return nil
    end
end

function TELEPORT_LOOKUP(me, map, x, y, direction)
    math.randomseed(seed())
    local rand_x = nil
    local rand_y = nil
    local new_direction = nil
    for i = 0, 3 do
        local case = (direction + i) % 4
        if case == DIRECTION_LEFT then
            rand_x = -1
            rand_y = 0
            new_direction = DIRECTION_RIGHT
        elseif case == DIRECTION_TOP then
            rand_x = 0
            rand_y = -1
            new_direction = DIRECTION_BOTTOM
        elseif case == DIRECTION_RIGHT then
            rand_x = 1
            rand_y = 0
            new_direction = DIRECTION_LEFT
        else
            rand_x = 0
            rand_y = 1
            new_direction = DIRECTION_TOP
        end

        if map:movable(me, x+rand_x, y+rand_y) then
            return x+rand_x, y+rand_y, new_direction
        end
    end

    return x, y, new_direction
end

function relative_buff_name(buff_name)
    local values = {
        {'혼마술', '저주', '귀염추혼소'},
        {'무장', '자동무장'},
        {'보호', '자동보호'},
        {'투명', '자동투명'}
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

function assert_map_debuff(me, you)
    local map = me:map()
    if map == nil then
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end

    local option = map:model():option()
    if you:is(OBJECT_TYPE_CHARACTER) then
        local pk = (option & MAP_OPTION_ENABLE_PK) == MAP_OPTION_ENABLE_PK
        if not pk then
            me:message('걸리지 않습니다.')
            return false
        end

        if you:assert_state(STATE_GHOST) then
            me:message('대상이 올바르지 않습니다.')
            return false
        end
    end

    return true
end

function assert_map_damage(me, you)
    local map = me:map()
    if map == nil then
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end

    local option = map:model():option()
    if you:is(OBJECT_TYPE_CHARACTER) then
        local pk = (option & MAP_OPTION_ENABLE_PK) == MAP_OPTION_ENABLE_PK
        if not pk then
            me:message('대상이 올바르지 않습니다.')
            return false
        end

        if you:assert_state(STATE_GHOST) then
            me:message('대상이 올바르지 않습니다.')
            return false
        end
    end

    return true
end

function spell_cast(me, you, spell, opts)
    opts = opts or {}

    local mp             = opts.mp             or 0
    local sound          = opts.sound
    local effect         = opts.effect
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action

    if not no_assert and me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end

    if me:mp() == 0 or me:mp() < mp then
        me:message("마력이 부족합니다.")
        return false
    end
    me:mp_down(mp)

    if you and effect then
        you:effect(effect)
    end
    if you and sound then
        you:sound(sound)
    end

    if spell then
        me:message(string.format("%s 외웠습니다.", name_with(spell:name())))
        if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
            you:message(string.format("%s님이 %s 외워주셨습니다.", me:name(), name_with(spell:name())))
        end
    end

    if default_action then
        me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    end

    return true
end

function buff_cast(me, you, spell, opts)
    opts = opts or {}

    local mp             = opts.mp             or 0
    local sound          = opts.sound          -- 기본 nil
    local effect         = opts.effect         -- 기본 nil
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action

    if me:is(OBJECT_TYPE_CHARACTER) then
        local error = me:assert_state(STATE_GHOST, STATE_RIDING)
        if error ~= nil then
            me:message(error)
            return false
        end
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end

    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if you:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        me:message('이미 걸려있습니다.')
        return false
    end

    if effect ~= nil then
        you:effect(effect)
    end
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end

    if default_action then
        me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    end
    return true
end

function debuff_cast(me, you, spell, opts)
    opts = opts or {}

    local mp             = opts.mp             or 0
    local sound          = opts.sound
    local effect         = opts.effect
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action

    if me:is(OBJECT_TYPE_CHARACTER) then
        local error = me:assert_state(STATE_GHOST, STATE_RIDING)
        if error ~= nil then
            me:message(error)
            return false
        end
    end

    if not assert_map_debuff(me, you) then
        return false
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end

    if me:is(OBJECT_TYPE_CHARACTER) and me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if you:isbuff(table.unpack(relative_buff_name(spell:name()))) then
        me:message('이미 걸려있습니다.')
        return false
    end

    if effect ~= nil then
        you:effect(effect)
    end
    
    if sound ~= nil then
        you:sound(sound)
    end
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE_CHARACTER) then
        if me:is(OBJECT_TYPE_CHARACTER) then
            you:message(string.format('%s님이 %s 걸었습니다.', me:name(), name_with(spell:name())))
        else
            you:message(string.format('%s %s 걸었습니다.', name_with(me:name(), '이', '가'), name_with(spell:name())))
        end
    end

    if default_action then
        me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    end
    return true
end


function attack_cast(me, you, spell, opts)
    opts = opts or {}

    local hp         = opts.hp         or 0
    local mp         = opts.mp         or 0
    local damage     = opts.damage     or 0
    local message    = opts.message    or ""
    local sound      = opts.sound
    local effect     = opts.effect
    local preprocess = opts.preprocess

    local map = me:map()
    if not map then
        return false
    end

    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end

    local option = map:model():option()
    if (option & MAP_OPTION_DISABLE_SPELL) == MAP_OPTION_DISABLE_SPELL then
        me:message("마력이 미치지 않습니다.")
        return false
    end

    if me:mp() == 0 or me:mp() < mp then
        me:message("마력이 부족합니다.")
        return false
    end
    me:mp_down(mp)

    if not you then
        return false
    end
    if type(you) == "userdata" then
        you = { you }
    end
    if #you == 0 then
        return false
    end

    if preprocess then
        for _, obj in pairs(you) do
            preprocess(me, obj)
        end
    end

    me:message(string.format("%s 외웠습니다.", name_with(spell:name())))
    me:action(ACTION_ATTACK, DURATION_ATTACK, 1)

    local pk      = (option & MAP_OPTION_ENABLE_PK) == MAP_OPTION_ENABLE_PK
    local damaged = false
    local rate    = me:skill_damage_rate() / 1000.0

    for _, obj in pairs(you) do
        if effect then
            obj:effect(effect)
        end
        if sound then
            obj:sound(sound)
        end

        if ((obj:is(OBJECT_TYPE_CHARACTER) and pk) or obj:is(OBJECT_TYPE_MOB)) then
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

function spell_damage(me, you, spell, opts)
    opts = opts or {}

    local damage = opts.damage or 0
    local mp     = opts.mp     or 0
    local sound  = opts.sound
    local effect = opts.effect

    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end

    if not you:is(OBJECT_TYPE_LIFE) then
        me:message("대상이 올바르지 않습니다.")
        return false
    end

    if not assert_map_damage(me, you) then
        return false
    end

    if me:mp() == 0 or me:mp() < mp then
        me:message("마력이 부족합니다.")
        return false
    end
    me:mp_down(mp)

    if effect then
        you:effect(effect)
    end
    if sound then
        you:sound(sound)
    end

    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)

    if you:is(OBJECT_TYPE_CHARACTER) then
        you:message(string.format("%s님이 %s 가합니다.",
            me:name(), name_with(spell:name())))
    end

    local rate = me:skill_damage_rate() / 1000.0
    you:damage(math.floor(damage * rate), me)

    return true
end

function spell_damage_near(me, spell, opts)
    opts = opts or {}
    local damage = opts.damage or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    me:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    local rate = me:skill_damage_rate() / 1000.0
    for _, you in pairs(near(me, OBJECT_TYPE_LIFE)) do
        if effect then you:effect(effect) end
        if you:is(OBJECT_TYPE_CHARACTER) then
            you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        you:damage(math.floor(damage * rate), me)
    end
    return true
end

function spell_damage_near_target(me, you, spell, opts)
    opts = opts or {}
    local damage = opts.damage or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    me:sound(sound)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    local rate = me:skill_damage_rate() / 1000.0
    local targets = near(you, OBJECT_TYPE_LIFE)
    table.insert(targets, you)
    for _, target in pairs(targets) do
        if target == me then goto CONTINUE_SPELL_DAMAGE_NEAR_TARGET end
        if effect then target:effect(effect) end
        if target:is(OBJECT_TYPE_CHARACTER) then
            target:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        target:damage(math.floor(damage * rate), me)
        ::CONTINUE_SPELL_DAMAGE_NEAR_TARGET::
    end
    return true
end

function spell_damage_area(me, you, spell, opts)
    opts = opts or {}
    local damage     = opts.damage or 0
    local hp         = opts.hp or 0
    local mp         = opts.mp or 0
    local sound      = opts.sound
    local effect     = opts.effect or {}

    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    if type(you) == 'userdata' then
        you = { you }
    end

    local rate = me:skill_damage_rate() / 1000.0
    for _, obj in pairs(you) do
        if obj:is(OBJECT_TYPE_CHARACTER) then
            obj:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        if effect.you then obj:effect(effect.you) end
        obj:damage(math.floor(damage * rate), me)
    end

    me:sound(sound)
    if effect.me then me:effect(effect.me) end
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)

    if hp > 0 then
        me:hp(me:hp() - hp)
    end

    return true
end

function spell_heal(me, you, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    if not spell_cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        return false
    end
    you:heal(hp)
    return true
end

function spell_heal_near(me, you, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if not you:is(OBJECT_TYPE_CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return false
    end
    me:mp_down(mp)
    local targets = near(you, OBJECT_TYPE_CHARACTER)
    table.insert(targets, you)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(targets) do
        ch:heal(hp)
        if effect then ch:effect(effect) end
        if sound then ch:sound(sound) end
        if me ~= ch then
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
    return true
end

function spell_heal_group(me, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    local map = me:map()
    if not map then
        return false
    end
    local group = me:group()
    if not group then
        me:message('가입된 그룹이 없습니다.')
        return false
    end
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return false
    end
    me:mp_down(mp)
    me:action(ACTION_CAST_SPELL, DURATION_SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(group:nears(map, { me:position() })) do
        if me ~= ch then
            ch:heal(hp)
            if effect then ch:effect(effect) end
            if sound then ch:sound(sound) end
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

function spell_disguise(me, mobs, name, spell, opts)
    opts = opts or {}
    local mp        = opts.mp or 0
    local sound     = opts.sound
    local effect    = opts.effect
    local buff_time = opts.buff_time or 0

    if me:is(OBJECT_TYPE_CHARACTER) then
        local err = me:assert_state(STATE_GHOST, STATE_RIDING)
        if err then
            me:message(err)
            return false
        end
    end

    local look = spell_disguise_look(me, mobs, name)
    if not look then
        return false
    end

    if buff_cast(me, me, spell, { mp = mp, sound = sound, effect = effect }) then
        me:buff(spell, buff_time)
        me:disguise(look)
        return true
    end

    return false
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

function spell_weapon_damage(me, mp, message, damage)
    local mp = 60
    if me:mp() == 0 or me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)

    local weapon = me:weapon()
    if weapon ~= nil and weapon:model():name() == message then
        me:message(string.format('%s 푸른 빛으로 빛납니다.', name_with(message, '이', '가')))
        me:weapon_damage(damage)
        return true
    else
        me:message('마법이 실패했습니다.')
        return false
    end
end