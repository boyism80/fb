local M = {}

local function execute_mob_spell_hit(me, you, spell)
    if not you:is(OBJECT_TYPE.MOB) then
        return true
    end

    local model = you:model()
    local id = model:id()
    local path = string.format('scripts/mob/%d.lua', id)
    local func = 'on_mob_spell_hit'
    local result = you:script(path, func, me, spell)
    if result == nil then
        return true -- continue
    else
        return result
    end
end

function M.boolean_random(percent)
    return math.random() < percent
end

-- Returns true if the target resists the effect.
function M.resisted(you, resist_type)
    if you == nil or not you:is(OBJECT_TYPE.LIFE) then
        return false
    end
    local r = you:resist(resist_type) or 0
    return M.boolean_random(r)
end

-- Forced position change; respects MOVE resist. Returns false if resisted.
function M.force_position(you, x, y)
    if M.resisted(you, RESIST.MOVE) then
        return false
    end
    you:position(x, y)
    return true
end

function M.divine_beast_spell(divine_beast, index)
    if divine_beast == DIVINE_BEAST.VERMILION_BIRD then
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
        
    elseif divine_beast == DIVINE_BEAST.WHITE_TIGER then
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
        
    elseif divine_beast == DIVINE_BEAST.BLACK_TORTOISE then
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
    elseif divine_beast == DIVINE_BEAST.AZURE_DRAGON then
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

function M.divine_beast_area_spell(divine_beast, index)
    if divine_beast == DIVINE_BEAST.VERMILION_BIRD then
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
        
    elseif divine_beast == DIVINE_BEAST.WHITE_TIGER then
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
        
    elseif divine_beast == DIVINE_BEAST.BLACK_TORTOISE then
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
    elseif divine_beast == DIVINE_BEAST.AZURE_DRAGON then
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

function M.teleport_lookup(me, map, x, y, direction)
    math.randomseed(seed())
    local rand_x = nil
    local rand_y = nil
    local new_direction = nil
    for i = 0, 3 do
        local case = (direction + i) % 4
        if case == DIRECTION.LEFT then
            rand_x = -1
            rand_y = 0
            new_direction = DIRECTION.RIGHT
        elseif case == DIRECTION.TOP then
            rand_x = 0
            rand_y = -1
            new_direction = DIRECTION.BOTTOM
        elseif case == DIRECTION.RIGHT then
            rand_x = 1
            rand_y = 0
            new_direction = DIRECTION.LEFT
        else
            rand_x = 0
            rand_y = 1
            new_direction = DIRECTION.TOP
        end
        
        if map:movable(me, x+rand_x, y+rand_y) then
            return x+rand_x, y+rand_y, new_direction
        end
    end
    
    return x, y, new_direction
end

M.relative_buff_groups = {
    {'혼마술', '저주', '귀염추혼소'},
    {'무장', '자동무장', '시약무장'},
    {'보호', '자동보호', '시약보호'},
    {'투명', '자동투명'},
    {'금수', '경수', '맹수', '야수', '의태시약'},
    {'용의제일주', '용의제이주', '용의제삼주', '용의제사주', '용의제오주', '용의제육주', '용의제칠주', '용의제팔주', '용의제구주'},
    {'용의제일노', '용의제이노', '용의제삼노', '용의제사노', '용의제오노', '용의제육노', '용의제칠노', '용의제팔노', '용의제구노'}
}

function M.relative_buff_name(buff_name)
    for _, names in pairs(M.relative_buff_groups) do
        for _, name in pairs(names) do
            if buff_name == name then
                return names
            end
        end
    end
    
    return {buff_name}
end

function M.near(me, type)
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

function M.attacker_blocks_pvp(me)
    return me ~= nil and me:is(OBJECT_TYPE.CHARACTER) and me:option(OPTION.PK_PROTECT)
end

function M.map_pk_enabled(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local model = map:model()
    local option = model:option()
    if (option & MAP_OPTION.ENABLE_PK) == MAP_OPTION.ENABLE_PK then
        return true
    end

    -- Siege maps: PK only while that divine beast's siege is active.
    -- Use siege_active() (sync) — never castle(), which can yield and break attack flow.
    if (option & MAP_OPTION.SIEGE_CASTLE) ~= MAP_OPTION.SIEGE_CASTLE then
        return false
    end

    local divine_beast = model:siege_castle()
    if divine_beast == nil then
        return false
    end

    return siege_active(divine_beast)
end

function M.map_siege_castle(me)
    local map = me:map()
    if map == nil then
        return false
    end
    local option = map:model():option()
    return (option & MAP_OPTION.SIEGE_CASTLE) == MAP_OPTION.SIEGE_CASTLE
end

function M.is_friendly_clan(me, you)
    if me == nil or you == nil then
        return false
    end
    if not me:is(OBJECT_TYPE.CHARACTER) or not you:is(OBJECT_TYPE.CHARACTER) then
        return false
    end

    local my_clan = me:clan()
    local your_clan = you:clan()
    if my_clan == nil or your_clan == nil then
        return false
    end

    if my_clan:id() == your_clan:id() then
        return true
    end

    return my_clan:is_allied(your_clan)
end

function M.blocks_siege_friendly_fire(me, you)
    return M.map_siege_castle(me) and M.is_friendly_clan(me, you)
end

function M.can_harm_character(me, you, pk, blocks_pvp)
    if you == nil or not you:is(OBJECT_TYPE.CHARACTER) then
        return false
    end

    -- Mob (or non-character) attackers keep previous AOE behavior: always may hit characters.
    if me == nil or not me:is(OBJECT_TYPE.CHARACTER) then
        return true
    end

    if pk == nil then
        pk = M.map_pk_enabled(me)
    end
    if not pk then
        return false
    end

    if blocks_pvp == nil then
        blocks_pvp = M.attacker_blocks_pvp(me)
    end
    if blocks_pvp then
        return false
    end

    if M.blocks_siege_friendly_fire(me, you) then
        return false
    end

    return true
end

function M.can_damage_life(me, you, pk, blocks_pvp)
    if you == nil or not you:is(OBJECT_TYPE.LIFE) then
        return false
    end

    if you:is(OBJECT_TYPE.MOB) then
        return true
    end

    return M.can_harm_character(me, you, pk, blocks_pvp)
end

function M.assert_map_debuff(me, you)
    local map = me:map()
    if map == nil then
        return false
    end
    
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    
    if you:is(OBJECT_TYPE.CHARACTER) then
        if not M.map_pk_enabled(me) then
            me:message('걸리지 않습니다.')
            return false
        end

        if M.attacker_blocks_pvp(me) then
            me:message('PK보호!')
            return false
        end
        
        if you:assert(STATE.GHOST) then
            me:message('대상이 올바르지 않습니다.')
            return false
        end
    end
    
    return true
end

function M.assert_map_damage(me, you)
    local map = me:map()
    if map == nil then
        return false
    end
    
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    
    if you:is(OBJECT_TYPE.CHARACTER) then
        if not M.map_pk_enabled(me) then
            me:message('대상이 올바르지 않습니다.')
            return false
        end

        if M.attacker_blocks_pvp(me) then
            me:message('PK보호!')
            return false
        end
        
        if you:assert(STATE.GHOST) then
            me:message('대상이 올바르지 않습니다.')
            return false
        end
    end
    
    return true
end

function M.cast(me, you, spell, opts)
    opts = opts or {}
    
    local mp             = opts.mp             or 0
    local sound          = opts.sound
    local effect         = opts.effect
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action
    
    if not no_assert and me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    
    if me:mp() < mp then
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
        if me ~= you and you:is(OBJECT_TYPE.CHARACTER) then
            you:message(string.format("%s님이 %s 외워주셨습니다.", me:name(), name_with(spell:name())))
        end
    end
    
    if default_action then
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end
    
    return true
end

function M.buff_cast(me, you, spell, opts)
    opts = opts or {}
    
    local mp             = opts.mp             or 0
    local sound          = opts.sound          -- 기본 nil
    local effect         = opts.effect         -- 기본 nil
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local error = me:assert(STATE.GHOST, STATE.RIDING)
        if error ~= nil then
            me:message(error)
            return false
        end
    end
    
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end
    
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    
    if you:isbuff(table.unpack(M.relative_buff_name(spell:name()))) then
        me:message('이미 걸려있습니다.')
        return false
    end
    
    if effect ~= nil then
        you:effect(effect)
    end
    you:sound(sound)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    if me ~= you and you:is(OBJECT_TYPE.CHARACTER) then
        you:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
    end
    
    if default_action then
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end

    return execute_mob_spell_hit(me, you, spell)
end

function M.debuff_cast(me, you, spell, opts)
    opts = opts or {}
    
    local mp             = opts.mp             or 0
    local sound          = opts.sound
    local effect         = opts.effect
    local no_assert      = opts.no_assert      or false
    local default_action = (opts.default_action == nil) and true or opts.default_action
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local error = me:assert(STATE.GHOST, STATE.RIDING)
        if error ~= nil then
            me:message(error)
            return false
        end
    end
    
    if not M.assert_map_debuff(me, you) then
        return false
    end
    
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message('걸리지 않습니다.')
        return false
    end
    
    if me:is(OBJECT_TYPE.CHARACTER) and me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    
    if you:isbuff(table.unpack(M.relative_buff_name(spell:name()))) then
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
    if me ~= you and you:is(OBJECT_TYPE.CHARACTER) then
        if me:is(OBJECT_TYPE.CHARACTER) then
            you:message(string.format('%s님이 %s 걸었습니다.', me:name(), name_with(spell:name())))
        else
            you:message(string.format('%s %s 걸었습니다.', name_with(me:name(), '이', '가'), name_with(spell:name())))
        end
    end
    
    if default_action then
        me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    end

    if M.resisted(you, RESIST.SPELL) then
        return false
    end
    if opts.resist ~= nil then
        local resists = opts.resist
        if type(resists) ~= 'table' then
            resists = { resists }
        end
        for _, resist_type in ipairs(resists) do
            if M.resisted(you, resist_type) then
                return false
            end
        end
    end

    return execute_mob_spell_hit(me, you, spell)
end


function M.attack_cast(me, you, spell, opts)
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
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    
    local option = map:model():option()
    if (option & MAP_OPTION.DISABLE_SPELL) == MAP_OPTION.DISABLE_SPELL then
        me:message("마력이 미치지 않습니다.")
        return false
    end
    
    if me:mp() < mp then
        me:message("마력이 부족합니다.")
        return false
    end
    me:mp_down(mp, false)
    
    if not you then
        me:update()
        return false
    end
    if type(you) == "userdata" then
        you = { you }
    end
    if #you == 0 then
        me:update()
        return false
    end
    
    if preprocess then
        for _, obj in pairs(you) do
            preprocess(me, obj)
        end
    end
    
    me:message(string.format("%s 외웠습니다.", name_with(spell:name())))
    me:action(ACTION.ATTACK, DURATION.ATTACK, 1)
    
    local pk = M.map_pk_enabled(me)
    local blocks_pvp = M.attacker_blocks_pvp(me)
    local damaged = false
    local skill_rate = me:skill_damage_rate() / 1000.0
    local targets = {}
    
    for _, obj in pairs(you) do
        if M.can_damage_life(me, obj, pk, blocks_pvp) then
            if effect then
                obj:effect(effect)
            end
            if sound then
                obj:sound(sound)
            end
            table.insert(targets, { obj, damage })
            damaged = true
        end
    end

    if #targets > 0 then
        me:damage_to(targets, { critical = false, rate = skill_rate })
    end
    
    if damaged then
        me:hp(math.max(10, me:hp() - hp), false)
    end
    me:update()
    
    me:chat(message, CHAT_TYPE.BLUE, false)
    return true
end

function M.damage(me, you, spell, opts)
    opts = opts or {}
    
    local damage = opts.damage or 0
    local mp     = opts.mp     or 0
    local sound  = opts.sound
    local effect = opts.effect
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    
    if not you:is(OBJECT_TYPE.LIFE) then
        me:message("대상이 올바르지 않습니다.")
        return false
    end
    
    if not M.assert_map_damage(me, you) then
        return false
    end
    
    if me:mp() < mp then
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
    
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    
    if you:is(OBJECT_TYPE.CHARACTER) then
        you:message(string.format("%s님이 %s 가합니다.", me:name(), name_with(spell:name())))
    end
    
    if execute_mob_spell_hit(me, you, spell) and not M.resisted(you, RESIST.SPELL) then
        me:damage_to(you, damage, { critical = false, rate = me:skill_damage_rate() / 1000.0, physical = false })
    end

    return true
end

function M.damage_near(me, spell, opts)
    opts = opts or {}
    local damage = opts.damage or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    me:sound(sound)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    local skill_rate = me:skill_damage_rate() / 1000.0
    local pk = M.map_pk_enabled(me)
    local blocks_pvp = M.attacker_blocks_pvp(me)
    local targets = {}
    for _, you in pairs(M.near(me, OBJECT_TYPE.LIFE)) do
        if M.can_damage_life(me, you, pk, blocks_pvp) then
            if effect then you:effect(effect) end
            if you:is(OBJECT_TYPE.CHARACTER) then
                you:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
            end
            
            if execute_mob_spell_hit(me, you, spell) and not M.resisted(you, RESIST.SPELL) then
                table.insert(targets, { you, damage })
            end
        end
    end
    if #targets > 0 then
        me:damage_to(targets, { critical = false, rate = skill_rate, physical = false })
    end
    return true
end

function M.damage_near_target(me, you, spell, opts)
    opts = opts or {}
    local damage = opts.damage or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect

    if you:is(OBJECT_TYPE.CHARACTER) then
        if not M.assert_map_damage(me, you) then
            return false
        end
    elseif not you:is(OBJECT_TYPE.LIFE) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end

    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp)
    me:sound(sound)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    local skill_rate = me:skill_damage_rate() / 1000.0
    local pk = M.map_pk_enabled(me)
    local blocks_pvp = M.attacker_blocks_pvp(me)
    local near_targets = M.near(you, OBJECT_TYPE.LIFE)
    table.insert(near_targets, you)
    local targets = {}
    for _, target in pairs(near_targets) do
        if target == me then 
            goto CONTINUE_SPELL_DAMAGE_NEAR_TARGET 
        end
        if not M.can_damage_life(me, target, pk, blocks_pvp) then
            goto CONTINUE_SPELL_DAMAGE_NEAR_TARGET
        end
        if effect then target:effect(effect) end
        if target:is(OBJECT_TYPE.CHARACTER) then
            target:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
        end
        
        if execute_mob_spell_hit(me, target, spell) and not M.resisted(target, RESIST.SPELL) then
            table.insert(targets, { target, damage })
        end
        ::CONTINUE_SPELL_DAMAGE_NEAR_TARGET::
    end
    if #targets > 0 then
        me:damage_to(targets, { critical = false, rate = skill_rate, physical = false })
    end
    return true
end

function M.damage_area(me, you, spell, opts)
    opts = opts or {}
    local damage     = opts.damage or 0
    local hp         = opts.hp or 0
    local mp         = opts.mp or 0
    local sound      = opts.sound
    local effect     = opts.effect or {}
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if me:mp() < mp then
        me:message('마력이 부족합니다.')
        return false
    end
    me:mp_down(mp, false)
    
    if type(you) == 'userdata' then
        you = { you }
    end

    local skill_rate = me:skill_damage_rate() / 1000.0
    local pk = M.map_pk_enabled(me)
    local blocks_pvp = M.attacker_blocks_pvp(me)
    local targets = {}
    for _, obj in pairs(you) do
        if M.can_damage_life(me, obj, pk, blocks_pvp) then
            if obj:is(OBJECT_TYPE.CHARACTER) then
                obj:message(string.format('%s님이 %s 가합니다.', me:name(), name_with(spell:name())))
            end

            if effect.you then 
                obj:effect(effect.you) 
            end
            
            if execute_mob_spell_hit(me, obj, spell) and not M.resisted(obj, RESIST.SPELL) then
                table.insert(targets, { obj, damage })
            end
        end
    end

    if #targets > 0 then
        me:damage_to(targets, { critical = false, rate = skill_rate, physical = false })
    end

    me:sound(sound)
    if effect.me then me:effect(effect.me) end
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    
    if hp > 0 then
        me:hp(me:hp() - hp, false)
    end
    me:update()
    
    return true
end

function M.heal(me, you, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    if not M.cast(me, you, spell, { mp = mp, sound = sound, effect = effect }) then
        return false
    end
    
    if execute_mob_spell_hit(me, you, spell) then
        you:heal(hp)
    end
    return true
end

function M.heal_near(me, you, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    if not you:is(OBJECT_TYPE.CHARACTER) then
        me:message('대상이 올바르지 않습니다.')
        return false
    end
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return false
    end
    me:mp_down(mp)
    local targets = M.near(you, OBJECT_TYPE.CHARACTER)
    table.insert(targets, you)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(targets) do
        if execute_mob_spell_hit(me, ch, spell) then
            ch:heal(hp)
        end
        if effect then
            ch:effect(effect)
        end
        if sound then
            ch:sound(sound)
        end
        if me ~= ch then
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
    return true
end

function M.heal_group(me, spell, opts)
    opts = opts or {}
    local hp = opts.hp or 0
    local mp = opts.mp or 0
    local sound = opts.sound
    local effect = opts.effect
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
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
    if me:mp() < mp then
        me:message('마력이 모자랍니다.')
        return false
    end
    me:mp_down(mp)
    me:action(ACTION.CAST_SPELL, DURATION.SPELL, 1)
    me:message(string.format('%s 외웠습니다.', name_with(spell:name())))
    for _, ch in pairs(group:nears(map, { me:position() })) do
        if me ~= ch then
            if execute_mob_spell_hit(me, ch, spell) then
                ch:heal(hp)
            end
            if effect then ch:effect(effect) end
            if sound then ch:sound(sound) end
            ch:message(string.format('%s님이 %s 외워주셨습니다.', me:name(), name_with(spell:name())))
        end
    end
    return true
end

function M.disguise_look(me, mobs, name)
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

function M.disguise(me, mobs, name, spell, opts)
    opts = opts or {}
    local mp        = opts.mp or 0
    local sound     = opts.sound
    local effect    = opts.effect
    local buff_time = opts.buff_time or 0
    
    if me:is(OBJECT_TYPE.CHARACTER) then
        local err = me:assert(STATE.GHOST, STATE.RIDING)
        if err then
            me:message(err)
            return false
        end
    end
    
    local look = M.disguise_look(me, mobs, name)
    if not look then
        return false
    end
    
    if M.buff_cast(me, me, spell, { mp = mp, sound = sound, effect = effect }) then
        me:buff(spell, buff_time)
        me:mimic({ disguise = look })
        return true
    end
    
    return false
end

function M.nears_exclude_item(me)
    local result = {}
    for _, obj in pairs(me:nears()) do
        if not obj:is(OBJECT_TYPE.ITEM) then
            table.insert(result, obj)
        end
    end
    return result
end

function M.front_exclude_item(me)
    local x, y = me:front_position()
    for _, obj in pairs(me:nears()) do
        if obj:is(OBJECT_TYPE.ITEM) then
            goto CONTINUE
        end

        local obj_x, obj_y = obj:position()
        if obj_x == x and obj_y == y then
            return obj
        end
        ::CONTINUE::
    end
    return nil
end

function M.front_obj(x, y, direction, step, objects, type)
    if direction == DIRECTION.LEFT then
        x = x-step
    elseif direction == DIRECTION.RIGHT then
        x = x+step
    elseif direction == DIRECTION.TOP then
        y = y-step
    else
        y = y+step
    end
    
    for _, obj in pairs(objects) do
        local obj_x, obj_y = obj:position()
        if x == obj_x and y == obj_y then
            if type == nil then
                if not obj:is(OBJECT_TYPE.ITEM) then
                    return obj
                end
            elseif obj:is(type) then
                return obj
            end
        end
    end
    
    return nil
end

function M.failed_attack(me)
    local message = '허공난무 흐미 실패닷'
    me:chat(message, CHAT_TYPE.BLUE)
    broadcast(string.format('[%s]: %s', me:name(), message), MESSAGE_TYPE.SHOUT, BROADCAST_TYPE.WORLD)
end

function M.weapon_damage(me, mp, message, damage)
    local mp = 60
    if me:mp() < mp then
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

return M
