-- Integration test helpers invoked from bots via:
--   /스크립트 test/integration <func> <step> [args...]
--
-- Design: every verify_* function is atomic — snapshot, act, and assert happen
-- inside a single call so no cross-invocation state (no _G) is required.
-- Results are reported as chat for scripts/integration/lib/script_step.lua:
--   CL:PASS:<step> [detail]
--   CL:FAIL:<step> [detail]
local spell = require('lib.spell')

local HIT = 500

local function report(me, ok, step, detail)
    local prefix = ok and 'CL:PASS:' or 'CL:FAIL:'
    local message = prefix .. tostring(step)
    if detail ~= nil and detail ~= '' then
        message = message .. ' ' .. detail
    end
    me:chat(message)
end

local function find_mob(me, oid)
    oid = tonumber(oid)
    if oid == nil then
        return nil
    end

    local map = me:map()
    if map == nil then
        return nil
    end

    for _, obj in pairs(map:objects(OBJECT_TYPE.MOB)) do
        if obj:oid() == oid then
            return obj
        end
    end
    return nil
end

local function harden(mob, extra_hp)
    mob:buff_hp(extra_hp or 10000)
    mob:hp(mob:maxhp())
end

local function explorer_snapshot(me, quest_ids)
    local quests = {}
    for id in tostring(quest_ids or ''):gmatch('%d+') do
        local q = me:quest(tonumber(id))
        local state = 'none'
        if q ~= nil then
            state = tostring(q:step()) .. (q:completed() and '!' or '')
        end
        table.insert(quests, id .. ':' .. state)
    end

    local counts = {}
    for _, item in pairs(me:items()) do
        local name = item:model():name()
        counts[name] = (counts[name] or 0) + item:count()
    end
    local items = {}
    for name, count in pairs(counts) do
        table.insert(items, name .. '=' .. count)
    end
    table.sort(items)

    local map = me:map()
    local map_id = map ~= nil and map:id() or 0
    return string.format('q=%s;m=%d;map=%d;i=%s',
        table.concat(quests, ','), me:money(), map_id, table.concat(items, ','))
end

-- spec: ';'-separated parts, applied in this order:
--   'clear'                   empty the inventory and remove every quest in lib.quest
--   'i=<name>=<count>,...'    create items
--   'q=<id>:<state>,...'      quest state as in snapshot ('none', '<step>', '<step>!')
--   'm=<money>'               set the money
--   'fill'                    fill every empty inventory slot with 목도
-- Returns an error message, or nil on success.
local function apply_state(me, spec)
    local parts = {}
    for part in tostring(spec or ''):gmatch('[^;]+') do
        parts[part:match('^(%a+)') or part] = part
    end

    if parts.clear ~= nil then
        for slot, item in pairs(me:items()) do
            me:rmitem(slot, item:count())
        end
        -- NPC scripts also touch quests the seed does not name, and those must not leak
        -- into the next path.
        for name, id in pairs(require('lib.quest')) do
            if name:match('^QUEST_') and type(id) == 'number' then
                me:remove_quest(id)
            end
        end
        -- Bots are created with a random gender, and some rewards depend on it.
        me:gender(GENDER.MALE)
    end
    if parts.i ~= nil then
        for name, count in parts.i:sub(3):gmatch('([^,=]+)=(%d+)') do
            if me:mkitem(name, tonumber(count)) == nil then
                return 'cannot create ' .. name
            end
        end
    end
    if parts.q ~= nil then
        for id, state in parts.q:sub(3):gmatch('(%d+):([^,]+)') do
            id = tonumber(id)
            me:remove_quest(id)
            if state ~= 'none' then
                local q = me:start_quest(id)
                if q == nil then
                    return 'cannot start quest ' .. id
                end
                q:step(tonumber(state:match('%d+')))
                if state:sub(-1) == '!' then
                    q:complete()
                end
            end
        end
    end
    if parts.m ~= nil then
        me:money(tonumber(parts.m:sub(3)))
    end
    if parts.fill ~= nil then
        for _ = 1, 52 do
            if me:mkitem('목도', 1) == nil then
                break
            end
        end
    end
    return nil
end

-- Links two parts to a body and sets its parts mode.
-- mode: 'PARTS' (default) or 'BODY'.

-- PARTS: hitting a part damages the part and forwards to the body.
-- BODY:  hitting a part damages only the body; part HP is unchanged.

-- Body with parts ignores direct hits.

-- PARTS death: kill p1 only → body survives; kill p2 → body dies.
-- BODY death: damage through part until body dies; part never dies alone first.

-- PHYSICAL / MAGIC damage resist (C++ calculate_damage path, fixed=false).
-- value '1.0' → HP unchanged; '0.0' → HP decreased.

-- SPELL gate: with SPELL=1.0 magic skill damage must not apply.
-- Mimics spell.damage's resisted(SPELL) check before damage_to.

-- Cross-type: PHYSICAL resist must not affect magic damage and vice versa.

return {
    link_parts = function(me, step, body_oid, p1_oid, p2_oid, mode)
        local body = find_mob(me, body_oid)
        local p1   = find_mob(me, p1_oid)
        local p2   = find_mob(me, p2_oid)
        if body == nil or p1 == nil or p2 == nil then
            return report(me, false, step, 'mob not found')
        end

        local parts_mode = (mode == 'BODY') and MOB_PARTS_MODE.BODY or MOB_PARTS_MODE.PARTS
        body:parts_mode(parts_mode)

        if body:parts(p1) == false then
            return report(me, false, step, 'link p1 failed')
        end
        if body:parts(p2) == false then
            return report(me, false, step, 'link p2 failed')
        end

        harden(p1, 10000)
        harden(p2, 10000)
        if parts_mode == MOB_PARTS_MODE.BODY then
            harden(body, 10000)
        else
            body:parts_mode(MOB_PARTS_MODE.PARTS) -- re-sync body HP from parts
        end

        report(me, true, step, string.format('body=%d p1=%d p2=%d', body:oid(), p1:oid(), p2:oid()))
    end,

    verify_parts_hit = function(me, step, body_oid, part_oid, mode)
        local body = find_mob(me, body_oid)
        local part = find_mob(me, part_oid)
        if body == nil or part == nil then
            return report(me, false, step, 'mob not found')
        end

        local before_part = part:hp()
        local before_body = body:hp()

        me:damage_to(part, HIT, { critical = false, rate = 1.0, physical = true, fixed = true })

        local after_part = part:hp()
        local after_body = body:hp()

        if mode == 'BODY' then
            if after_body >= before_body then
                return report(me, false, step, 'body HP not decreased')
            end
            if after_part ~= before_part then
                return report(me, false, step, 'part HP changed in BODY mode')
            end
        else
            if after_part >= before_part then
                return report(me, false, step, 'part HP not decreased')
            end
            if after_body >= before_body then
                return report(me, false, step, 'body HP not forwarded')
            end
        end

        report(me, true, step)
    end,

    verify_body_direct_ignored = function(me, step, body_oid)
        local body = find_mob(me, body_oid)
        if body == nil then
            return report(me, false, step, 'mob not found')
        end

        local parts = body:parts()
        if parts == nil or #parts == 0 then
            return report(me, false, step, 'body has no parts')
        end

        local before = body:hp()
        me:damage_to(body, HIT, { critical = false, rate = 1.0, physical = true, fixed = true })
        if body:hp() ~= before then
            return report(me, false, step, 'body took direct damage')
        end
        report(me, true, step)
    end,

    verify_parts_death = function(me, step, body_oid, p1_oid, p2_oid, mode)
        local body = find_mob(me, body_oid)
        local p1   = find_mob(me, p1_oid)
        local p2   = find_mob(me, p2_oid)
        if body == nil or p1 == nil or p2 == nil then
            return report(me, false, step, 'mob not found')
        end

        if mode == 'BODY' then
            while body:hp() > 0 do
                me:damage_to(p1, HIT, { critical = false, rate = 1.0, physical = true, fixed = true })
                if find_mob(me, p1_oid) == nil and find_mob(me, body_oid) ~= nil then
                    return report(me, false, step, 'part died independently in BODY mode')
                end
                body = find_mob(me, body_oid)
                p1   = find_mob(me, p1_oid)
                if body == nil then
                    break
                end
            end
            if find_mob(me, body_oid) ~= nil then
                return report(me, false, step, 'body still alive')
            end
            return report(me, true, step)
        end

        -- PARTS: drain p1 completely.
        while p1 ~= nil and p1:hp() > 0 do
            me:damage_to(p1, HIT, { critical = false, rate = 1.0, physical = true, fixed = true })
            p1 = find_mob(me, p1_oid)
        end
        if find_mob(me, p1_oid) ~= nil then
            return report(me, false, step, 'part1 still alive')
        end
        if find_mob(me, body_oid) == nil then
            return report(me, false, step, 'body died before all parts')
        end

        p2 = find_mob(me, p2_oid)
        while p2 ~= nil and p2:hp() > 0 do
            me:damage_to(p2, HIT, { critical = false, rate = 1.0, physical = true, fixed = true })
            p2 = find_mob(me, p2_oid)
        end
        if find_mob(me, body_oid) ~= nil then
            return report(me, false, step, 'body still alive after all parts')
        end

        report(me, true, step)
    end,

    verify_damage_resist = function(me, step, oid, resist_type, value)
        local mob = find_mob(me, oid)
        if mob == nil then
            return report(me, false, step, 'mob not found')
        end

        local rtype = RESIST[resist_type]
        if rtype == nil then
            return report(me, false, step, 'unknown resist type')
        end

        local physical = (resist_type == 'PHYSICAL')
        mob:buff_resist(rtype, tonumber(value) or 0.0)
        harden(mob, 100000)

        local before = mob:hp()
        me:damage_to(mob, 2000, { critical = false, rate = 1.0, physical = physical, fixed = false })
        local after = mob:hp()

        local expect_block = (tonumber(value) or 0) >= 1.0
        if expect_block then
            if after ~= before then
                return report(me, false, step, string.format('expected block before=%s after=%s', before, after))
            end
        else
            if after >= before then
                return report(me, false, step, string.format('expected damage before=%s after=%s', before, after))
            end
        end

        report(me, true, step)
    end,

    verify_spell_gate = function(me, step, oid, value)
        local mob = find_mob(me, oid)
        if mob == nil then
            return report(me, false, step, 'mob not found')
        end

        mob:buff_resist(RESIST.SPELL, tonumber(value) or 0.0)
        harden(mob, 100000)

        local before = mob:hp()
        if not spell.resisted(mob, RESIST.SPELL) then
            me:damage_to(mob, 2000, { critical = false, rate = 1.0, physical = false, fixed = false })
        end
        local after = mob:hp()

        local expect_block = (tonumber(value) or 0) >= 1.0
        if expect_block then
            if after ~= before then
                return report(me, false, step, 'SPELL did not block')
            end
        else
            if after >= before then
                return report(me, false, step, 'SPELL blocked unexpectedly')
            end
        end

        report(me, true, step)
    end,

    verify_cross_type = function(me, step, oid)
        local mob = find_mob(me, oid)
        if mob == nil then
            return report(me, false, step, 'mob not found')
        end

        harden(mob, 100000)

        mob:buff_resist(RESIST.PHYSICAL, 1.0)
        mob:buff_resist(RESIST.MAGIC, 0.0)
        local before = mob:hp()
        me:damage_to(mob, 2000, { critical = false, rate = 1.0, physical = false, fixed = false })
        if mob:hp() >= before then
            return report(me, false, step, 'PHYSICAL resist blocked magic')
        end

        mob:buff_resist(RESIST.PHYSICAL, 0.0)
        mob:buff_resist(RESIST.MAGIC, 1.0)
        harden(mob, 100000)
        before = mob:hp()
        me:damage_to(mob, 2000, { critical = false, rate = 1.0, physical = true, fixed = false })
        if mob:hp() >= before then
            return report(me, false, step, 'MAGIC resist blocked physical')
        end

        -- SPELL=1.0 must not block physical damage_to (gate is Lua-only).
        mob:buff_resist(RESIST.SPELL, 1.0)
        mob:buff_resist(RESIST.PHYSICAL, 0.0)
        harden(mob, 100000)
        before = mob:hp()
        me:damage_to(mob, 2000, { critical = false, rate = 1.0, physical = true, fixed = false })
        if mob:hp() >= before then
            return report(me, false, step, 'SPELL resist blocked physical')
        end

        report(me, true, step)
    end,

    verify_gate = function(me, step, resist_type, value)
        local rtype = RESIST[resist_type]
        if rtype == nil then
            return report(me, false, step, 'unknown resist type: ' .. tostring(resist_type))
        end

        local x, y   = me:position()
        local target = me:spawn_mob('다람쥐', x, y, false)
        if target == nil then
            return report(me, false, step, 'spawn failed')
        end

        target:buff_resist(rtype, tonumber(value) or 0.0)

        local final = target:resist(rtype)
        local expected
        if final >= 1.0 then
            expected = true
        elseif final <= 0.0 then
            expected = false
        else
            target:destroy()
            return report(me, true, step, 'non-deterministic range skipped')
        end

        local ok = true
        for _ = 1, 50 do
            if spell.resisted(target, rtype) ~= expected then
                ok = false
                break
            end
        end

        target:destroy()
        report(me, ok, step)
    end,

    verify_clamp = function(me, step)
        local x, y   = me:position()
        local target = me:spawn_mob('다람쥐', x, y, false)
        if target == nil then
            return report(me, false, step, 'spawn failed')
        end

        target:buff_resist(RESIST.PHYSICAL, 2.0)
        local hi = target:resist(RESIST.PHYSICAL)
        target:buff_resist(RESIST.PHYSICAL, -1.0)
        local lo = target:resist(RESIST.PHYSICAL)

        target:destroy()

        if hi == 1.0 and lo == 0.0 then
            report(me, true, step)
        else
            report(me, false, step, string.format('hi=%.2f lo=%.2f', hi, lo))
        end
    end,

    verify_link_reject = function(me, step, case)
        local x, y = me:position()
        local ok   = false

        if case == 'self' then
            local b = me:spawn_mob('다람쥐', x, y, false)
            ok = (b:parts(b) == false)
            b:destroy()
        elseif case == 'already_linked' then
            local a = me:spawn_mob('다람쥐', x, y, false)
            local b = me:spawn_mob('다람쥐', x, y, false)
            local p = me:spawn_mob('토끼', x, y, false)
            a:parts(p)
            ok = (b:parts(p) == false)
            a:destroy()
            b:destroy()
        elseif case == 'nested' then
            local a = me:spawn_mob('다람쥐', x, y, false)
            local b = me:spawn_mob('다람쥐', x, y, false)
            local p = me:spawn_mob('토끼', x, y, false)
            a:parts(p)
            ok = (b:parts(a) == false)
            a:destroy()
            b:destroy()
        elseif case == 'part_as_body' then
            local a = me:spawn_mob('다람쥐', x, y, false)
            local p = me:spawn_mob('토끼', x, y, false)
            local q = me:spawn_mob('토끼', x, y, false)
            a:parts(p)
            ok = (p:parts(q) == false)
            a:destroy()
            q:destroy()
        else
            return report(me, false, step, 'unknown case: ' .. tostring(case))
        end

        report(me, ok, step)
    end,

    explode = function(me, step)
        error('integration boom')
    end,

    verify_super_hide = function(me, step, expect)
        local want   = expect == '1'
        local hidden = me:super_hide() == true
        local cloack = me:state() == STATE.CLOACK
        if want then
            if hidden == false or cloack == false then
                return report(me, false, step, string.format('hide=%s cloack=%s', tostring(hidden), tostring(cloack)))
            end
        else
            if hidden or cloack then
                return report(me, false, step, 'still hidden')
            end
        end
        report(me, true, step)
    end,

    clear_super_hide = function(me, step)
        me:super_hide(false)
        me:state(STATE.NORMAL)
        if me:super_hide() == true or me:state() == STATE.CLOACK then
            return report(me, false, step, 'still hidden')
        end
        report(me, true, step)
    end,

    verify_weapon_damage = function(me, step, expected)
        local want = tonumber(expected)
        local now  = me:weapon_damage()
        if now ~= want then
            return report(me, false, step, string.format('weapon_damage=%s expected=%s', tostring(now), tostring(want)))
        end
        if want > 0 then
            for _ = 1, 20 do
                local damage = me:normal_attack_damage(MOB_SIZE.SMALL)
                if damage <= want then
                    return report(me, false, step, string.format('damage=%s bonus=%s', tostring(damage), tostring(want)))
                end
            end
        end
        report(me, true, step)
    end,

    verify_map_instance = function(me, step, expected)
        local map = me:map()
        if map == nil then
            return report(me, false, step, 'no map')
        end
        local now = map:instance()
        if now ~= (expected == '1') then
            return report(me, false, step, string.format('instance=%s slot=%s', tostring(now), tostring(map:slot())))
        end
        report(me, true, step)
    end,

    set_role = function(me, step, name, role)
        local you = name2ch(name)
        if you == nil then
            return report(me, false, step, 'missing')
        end
        local value = tonumber(role)
        you:role(value)
        if you:role() ~= value then
            return report(me, false, step, 'role mismatch')
        end
        report(me, true, step)
    end,

    verify_demoted = function(me, step, name)
        local you = name2ch(name)
        if you == nil then
            return report(me, false, step, 'missing')
        end
        if you:super_hide() == true or you:state() == STATE.CLOACK then
            return report(me, false, step, 'still hidden')
        end
        report(me, true, step)
    end,

    grant_spell = function(me, step, name, spell_name)
        local you = name2ch(name)
        if you == nil then
            return report(me, false, step, 'missing')
        end
        if you:mkspell(spell_name) == nil then
            return report(me, false, step, 'mkspell failed')
        end
        report(me, true, step)
    end,

    verify_no_buff = function(me, step, name, spell_name)
        local you = name2ch(name)
        if you == nil then
            return report(me, false, step, 'missing')
        end
        local names = {}
        for _, buff in pairs(you:buffs()) do
            table.insert(names, buff:model():name())
        end
        local listed = table.concat(names, ',')
        if you:isbuff(spell_name) then
            return report(me, false, step, listed)
        end
        report(me, true, step, listed)
    end,

    -- kind: phydef | derate | rates. Applies the buff and checks the stat comes back on unbuff.
    verify_buff_roundtrip = function(me, step, spell_name, kind)
        local function read_stat()
            if kind == 'phydef' then
                return me:buff_phydef()
            elseif kind == 'derate' then
                return me:damage_derate()
            elseif kind == 'rates' then
                return me:damage_rate() + me:skill_damage_rate()
            end
            return nil
        end

        if name2spell(spell_name) == nil then
            return report(me, false, step, 'no spell')
        end

        local before = read_stat()
        if before == nil then
            return report(me, false, step, 'unknown kind')
        end

        me:buff(spell_name, 30)
        local mid = read_stat()
        me:unbuff(spell_name)
        local after = read_stat()

        if mid == before then
            return report(me, false, step, 'stat unchanged while buffed')
        end
        if after ~= before then
            return report(me, false, step, string.format('before=%s mid=%s after=%s', before, mid, after))
        end
        report(me, true, step)
    end,

    -- Recasts right after the buff expires and before the buff timer removes it.
    verify_buff_recast = function(me, step, spell_name, kind)
        local function read_stat()
            if kind == 'phydef' then
                return me:buff_phydef()
            elseif kind == 'derate' then
                return me:damage_derate()
            elseif kind == 'rates' then
                return me:damage_rate() + me:skill_damage_rate()
            end
            return nil
        end

        if name2spell(spell_name) == nil then
            return report(me, false, step, 'no spell')
        end

        local before = read_stat()
        if before == nil then
            return report(me, false, step, 'unknown kind')
        end

        local buff = me:buff(spell_name, 30)
        if buff == nil then
            return report(me, false, step, 'buff failed')
        end
        local once = read_stat()
        buff:time(0)
        me:buff(spell_name, 30)
        local recast = read_stat()
        me:unbuff(spell_name)
        local after = read_stat()

        if recast ~= once then
            return report(me, false, step, string.format('once=%s recast=%s', once, recast))
        end
        if after ~= before then
            return report(me, false, step, string.format('before=%s after=%s', before, after))
        end
        report(me, true, step)
    end,

    verify_derate = function(me, step, expected)
        local now  = me:damage_derate()
        local want = tonumber(expected)
        if now ~= want then
            return report(me, false, step, string.format('derate=%s expected=%s', tostring(now), tostring(want)))
        end
        report(me, true, step)
    end,

    snapshot_derate = function(me, step)
        report(me, true, step, tostring(me:damage_derate()))
    end,

    snapshot_buff_phydef = function(me, step)
        report(me, true, step, tostring(me:buff_phydef()))
    end,

    verify_buff_phydef = function(me, step, expected)
        local now  = me:buff_phydef()
        local want = tonumber(expected)
        if now ~= want then
            return report(me, false, step, string.format('phydef=%s expected=%s', tostring(now), tostring(want)))
        end
        report(me, true, step)
    end,

    verify_concast_heal = function(me, step, spell_name)
        if name2spell(spell_name) == nil then
            return report(me, false, step, 'no spell')
        end
        if me:isbuff(spell_name) ~= true then
            me:buff(spell_name, 30)
        end

        local maxhp = me:maxhp()
        me:hp(math.max(1, maxhp // 2))
        local before = me:hp()
        sleep(2200)
        local after = me:hp()
        me:unbuff(spell_name)

        if after <= before then
            return report(me, false, step, string.format('before=%s after=%s', before, after))
        end
        report(me, true, step)
    end,

    verify_mimic = function(me, step)
        if me:isbuff('의태') ~= true then
            return report(me, false, step, 'no buff')
        end
        report(me, true, step)
    end,

    -- mode 'warp': the caller already stands in 견우직녀의집, so the warp stays on this server.
    -- mode 'blocked': 견우직녀의집 is hosted by a server that is not running, so the warp is refused.
    verify_reunion = function(me, step, mode)
        local origin = me:map()
        if origin == nil or origin:model() == nil then
            return report(me, false, step, 'no map')
        end
        local origin_name = origin:model():name()
        if mode == 'warp' and origin_name ~= '견우직녀의집' then
            return report(me, false, step, 'warp mode must start in 견우직녀의집, map=' .. origin_name)
        end

        local hair = me:hair()
        me:buff('견우직녀축복', 30)
        me:unbuff('견우직녀축복')

        -- The warp in on_unbuff runs on the map thread, so the new position shows up after a short delay.
        local now_name = ''
        local x, y = 0, 0
        for _ = 1, 10 do
            local now = me:map()
            if now ~= nil and now:model() ~= nil then
                now_name = now:model():name()
            end
            x, y = me:position()
            if mode == 'warp' and x >= 7 and x <= 12 and y >= 6 and y <= 15 then
                break
            end
            sleep(100)
        end

        local expect_hair = 89
        if me:gender() == GENDER.MALE then
            expect_hair = 19
        end
        local hair_now = me:hair()
        me:hair(hair)

        local ok = hair_now == expect_hair
        if mode == 'warp' then
            ok = ok and now_name == '견우직녀의집' and x >= 7 and x <= 12 and y >= 6 and y <= 15
        else
            ok = ok and now_name == origin_name
        end

        if ok then
            report(me, true, step)
        else
            report(me, false, step, string.format('mode=%s map=%s pos=(%d,%d) hair=%s',
                tostring(mode), now_name, x, y, tostring(hair_now)))
        end
    end,

    -- state: 'none' removes the quest, '<n>' starts it at step n, '<n>:done' also completes it.
    set_quest = function(me, step, id, state)
        id = tonumber(id)
        if id == nil or state == nil then
            return report(me, false, step, 'usage: set_quest <step> <id> <none|n|n:done>')
        end

        me:remove_quest(id)
        if state == 'none' then
            return report(me, true, step)
        end

        local qstep, done = state:match('^(%d+)(.*)$')
        if qstep == nil or (done ~= '' and done ~= ':done') then
            return report(me, false, step, 'bad quest state ' .. state)
        end
        local q = me:start_quest(id)
        if q == nil then
            return report(me, false, step, 'cannot start quest ' .. tostring(id))
        end
        q:step(tonumber(qstep))
        if done == ':done' then
            q:complete()
        end
        report(me, true, step)
    end,

    set_money = function(me, step, value)
        me:money(tonumber(value))
        report(me, true, step)
    end,

    -- expected: 'none', '<n>' (in progress at step n) or '<n>:done' (completed at step n).
    verify_quest = function(me, step, id, expected)
        local q = me:quest(tonumber(id))
        local actual = 'none'
        if q ~= nil then
            actual = tostring(q:step())
            if q:completed() then
                actual = actual .. ':done'
            end
        end

        if actual == expected then
            report(me, true, step)
        else
            report(me, false, step,
                string.format('quest %s is %s, expected %s', tostring(id), actual, tostring(expected)))
        end
    end,

    -- spec: comma-separated 'name=count' pairs, e.g. '홍옥=0,정화비서=1'. 'money=n' checks the money.
    verify_items = function(me, step, spec)
        local mismatches = {}
        for name, expected in tostring(spec):gmatch('([^,=]+)=(%d+)') do
            local actual = 0
            if name == 'money' then
                actual = me:money()
            else
                for _, item in pairs(me:items(name) or {}) do
                    actual = actual + item:count()
                end
            end
            if actual ~= tonumber(expected) then
                table.insert(mismatches, string.format('%s=%d(expected %s)', name, actual, expected))
            end
        end

        if #mismatches == 0 then
            report(me, true, step)
        else
            report(me, false, step, table.concat(mismatches, ','))
        end
    end,

    -- Each map:doors() entry must be its own door: toggling one entry flips exactly one entry.
    -- Checks the first few doors that can toggle and restores each one.
    verify_doors_unique = function(me, step)
        local map = me:map()
        if map == nil then
            return report(me, false, step, 'no map')
        end

        local doors = map:doors()
        local checked = 0
        for i, door in ipairs(doors) do
            if checked >= 5 then
                break
            end

            local before = {}
            for j, other in ipairs(doors) do
                before[j] = other:opened()
            end

            if door:toggle() ~= nil then
                local flipped = 0
                for j, other in ipairs(doors) do
                    if other:opened() ~= before[j] then
                        flipped = flipped + 1
                    end
                end
                door:toggle()
                checked = checked + 1

                if flipped ~= 1 then
                    return report(me, false, step, string.format('door %d flipped %d entries', i, flipped))
                end
            end
        end

        if checked == 0 then
            return report(me, false, step, 'no door toggled')
        end
        report(me, true, step, string.format('doors=%d checked=%d', #doors, checked))
    end,

    -- key: an OPTION name, e.g. 'WHISPER'. expected: 'true' or 'false'.
    verify_option = function(me, step, key, expected)
        local actual = tostring(me:option(OPTION[key]))
        if actual == expected then
            report(me, true, step)
        else
            report(me, false, step, string.format('%s is %s, expected %s', tostring(key), actual, tostring(expected)))
        end
    end,

    -- Removes a mob the way a despawn or a script does, without killing it.
    destroy_mob = function(me, step, oid)
        local mob = find_mob(me, oid)
        if mob == nil then
            return report(me, false, step, 'mob not found')
        end
        mob:destroy()
        report(me, true, step)
    end,

    map_active = function(me, step, map_id)
        local map = id2map(tonumber(map_id))
        report(me, map ~= nil and map:active() == true, step)
    end,

    move_to_map = function(me, step, map_id, x, y)
        local map = id2map(tonumber(map_id))
        if map == nil then
            return report(me, false, step, 'no map ' .. tostring(map_id))
        end
        report(me, me:map(map, { tonumber(x), tonumber(y) }) == true, step)
    end,

    -- beast: 주작/현무/청룡/백호. active: 'true' or 'false'. owner: 'mine' or 'not_mine' against the caller's clan.
    verify_siege = function(me, step, beast, active, owner)
        local divine_beast = require('schedule.siege').NAME_TO_DIVINE_BEAST[beast]
        local entity = divine_beast and castle(divine_beast) or nil
        if entity == nil then
            return report(me, false, step, 'no castle ' .. tostring(beast))
        end

        local clan = me:clan()
        local mine = clan ~= nil and entity:owner_clan() == clan:id()
        local actual_active = tostring(entity:siege_active())
        local actual_owner = mine and 'mine' or 'not_mine'
        if actual_active == active and actual_owner == owner then
            report(me, true, step)
        else
            report(me, false, step,
                string.format('active=%s owner=%s, expected active=%s owner=%s', actual_active, actual_owner, active,
                    owner))
        end
    end,

    verify_state_machine = function(me, step)
        if state_machine('magpie_bridge') == nil then
            report(me, false, step, 'magpie_bridge machine is missing')
            return
        end

        local sm = state_machine('integration_probe')
        if sm == nil then
            report(me, false, step, 'integration_probe machine is missing')
            return
        end

        sm:call('on_probe_start')
        sleep(1800)
        local ticks = sm:get('ticks') or 0
        sm:call('on_probe_stop')
        sleep(1200)
        local stopped = sm:get('stopped_ticks')
        local after = sm:get('ticks')

        if sm:get('called') ~= true then
            report(me, false, step, 'call did not run the hook')
        elseif ticks < 2 then
            report(me, false, step, string.format('every ran %d times in 1.8s', ticks))
        elseif stopped == nil or after ~= stopped then
            report(me, false, step,
                string.format('cancel left the timer running stopped=%s after=%s', tostring(stopped), tostring(after)))
        else
            sm:call('on_probe_hold', 1000)
            sleep(100)
            sm:call('on_probe_peek')
            sleep(1500)
            local peeked = sm:get('peeked')
            if peeked ~= 'after_hold' then
                report(me, false, step, 'a hook ran while another hook was suspended peeked=' .. tostring(peeked))
            else
                report(me, true, step)
            end
        end
    end,

    magpie_walk_without_support = function(me, step, enabled)
        if enabled == 'true' then
            property('magpie_bridge.walk_without_support', true)
        else
            property('magpie_bridge.walk_without_support', nil)
        end
        report(me, true, step)
    end,

    verify_magpie = function(me, step, spec)
        local sm = state_machine('magpie_bridge')
        if sm == nil then
            return report(me, false, step, 'magpie_bridge machine is missing')
        end

        local bridge = require('lib.magpie_bridge')
        local bridge_map = id2map(bridge.MAP_ID)
        local mismatches = {}
        for key, expected in tostring(spec):gmatch('([^,=]+)=([^,]+)') do
            local actual = nil
            local mob_id = tonumber(string.match(key, '^mob(%d+)$'))
            if mob_id ~= nil then
                local count = 0
                for _, mob in pairs(bridge_map ~= nil and bridge_map:objects(OBJECT_TYPE.MOB) or {}) do
                    if mob:model():id() == mob_id then
                        count = count + 1
                    end
                end
                actual = tostring(count)
            elseif key == 'remaining' then
                local remaining = bridge.bridge_build_remaining()
                if remaining == nil then
                    actual = 'none'
                elseif remaining > 0 then
                    actual = 'positive'
                else
                    actual = 'zero'
                end
            else
                actual = tostring(sm:get(key))
            end
            if actual ~= expected then
                table.insert(mismatches, string.format('%s=%s(expected %s)', key, actual, expected))
            end
        end

        if #mismatches == 0 then
            report(me, true, step)
        else
            report(me, false, step, table.concat(mismatches, ','))
        end
    end,

    verify_none_exp_cap = function(me, step)
        local cls = me:class()
        local level = me:level()
        local exp = me:exp()
        local above_cap = 100000

        me:class(CLASS.NONE)
        me:exp(above_cap)
        me:exchange({}, { exp = 10 })
        local after = me:exp()

        me:exp(exp)
        me:level(level)
        me:class(cls)

        if after == above_cap then
            report(me, true, step)
        else
            report(me, false, step,
                string.format('exp %d -> %s after a gain above the class NONE cap', above_cap, tostring(after)))
        end
    end,

    verify_potion_refund = function(me, step, name)
        local function count_of()
            local total = 0
            for _, item in pairs(me:items(name) or {}) do
                total = total + item:count()
            end
            return total
        end

        local mp = me:mp()
        me:mp(0)
        local item = me:mkitem(name, 1)
        if item == nil then
            me:mp(mp)
            return report(me, false, step, 'could not make ' .. name)
        end

        local before = count_of()
        me:active(item)
        sleep(500)
        local after = count_of()
        me:mp(mp)
        for _, held in pairs(me:items(name) or {}) do
            me:rmitem(held, held:count())
        end

        if after == before then
            report(me, true, step)
        else
            report(me, false, step, string.format('%s count %d -> %d after a failed use', name, before, after))
        end
    end,

    set_ghost = function(me, step, enabled)
        if enabled == 'true' then
            me:state(STATE.GHOST)
        else
            me:state(STATE.NORMAL)
        end
        report(me, true, step)
    end,

    verify_caster_state = function(me, step)
        local sp = name2spell('귀염추혼소')
        if sp == nil then
            return report(me, false, step, 'no spell')
        end

        local mp = me:mp()
        local failures = {}
        for _, state in ipairs({ STATE.GHOST, STATE.RIDING }) do
            me:state(state)
            me:mp(me:maxmp())
            local before = me:mp()
            if spell.damage_near(me, sp, { mp = 1 }) ~= false then
                table.insert(failures, state .. ':damage_near')
            end
            if spell.damage_near_target(me, me, sp, { mp = 1 }) ~= false then
                table.insert(failures, state .. ':damage_near_target')
            end
            if #spell.area_debuff_cast(me, sp, { mp = 1 }) ~= 0 then
                table.insert(failures, state .. ':area_debuff_cast')
            end
            if me:mp() ~= before then
                table.insert(failures, string.format('%d:mp %d -> %d', state, before, me:mp()))
            end
        end
        me:state(STATE.NORMAL)
        me:mp(mp)

        if #failures == 0 then
            report(me, true, step)
        else
            report(me, false, step, table.concat(failures, ','))
        end
    end,

    verify_uncloak = function(me, step)
        local failures = {}
        local function expect(label, want)
            if me:state() ~= want then
                table.insert(failures, string.format('%s=%d', label, me:state()))
            end
        end

        me:state(STATE.RIDING)
        me:cast('자동투명')
        sleep(100)
        if me:isbuff('자동투명') then
            table.insert(failures, 'riding_auto_cloak_cast')
        end
        expect('riding_cast', STATE.RIDING)
        me:buff('자동투명', 30)
        sleep(100)
        expect('riding_auto_buff', STATE.RIDING)
        me:unbuff('자동투명')
        expect('riding_auto_unbuff', STATE.RIDING)
        me:buff('투명', 30)
        me:unbuff('투명')
        expect('riding_cloak_unbuff', STATE.RIDING)

        me:state(STATE.NORMAL)
        me:buff('자동투명', 30)
        sleep(100)
        expect('auto_buff', STATE.HALF_CLOACK)
        me:unbuff('자동투명')
        expect('auto_unbuff', STATE.NORMAL)

        me:state(STATE.CLOACK)
        me:buff('투명', 30)
        me:unbuff('투명')
        expect('cloak_unbuff', STATE.NORMAL)

        me:unbuff('자동투명')
        me:unbuff('투명')
        me:state(STATE.NORMAL)

        if #failures == 0 then
            report(me, true, step)
        else
            report(me, false, step, table.concat(failures, ','))
        end
    end,

    verify_heal_ghost = function(me, step, name)
        local you = name2ch(name)
        if you == nil then
            return report(me, false, step, 'missing')
        end
        local sp = name2spell('신령의기원')
        if sp == nil then
            return report(me, false, step, 'no spell')
        end

        local failures = {}
        local maxhp = you:maxhp()
        you:state(STATE.GHOST)
        you:hp(1)
        if spell.heal(me, you, sp, { hp = 100 }) ~= false then
            table.insert(failures, 'heal')
        end
        if spell.heal_near(me, you, sp, { hp = 100 }) ~= false then
            table.insert(failures, 'heal_near')
        end
        if you:hp() ~= 1 then
            table.insert(failures, string.format('ghost hp 1 -> %d', you:hp()))
        end

        you:state(STATE.NORMAL)
        you:hp(math.max(1, maxhp // 2))
        local before = you:hp()
        if spell.heal(me, you, sp, { hp = 1 }) ~= true or you:hp() <= before then
            table.insert(failures, string.format('alive hp %d -> %d', before, you:hp()))
        end
        you:hp(maxhp)

        if #failures == 0 then
            report(me, true, step)
        else
            report(me, false, step, table.concat(failures, ','))
        end
    end,

    remove_npc = function(me, step, name)
        local removed = 0
        for _, obj in pairs(me:map():objects(OBJECT_TYPE.NPC)) do
            if obj:name() == name then
                obj:destroy()
                removed = removed + 1
            end
        end
        report(me, removed > 0, step, tostring(removed))
    end,

    revive_with_hp = function(me, step, hp)
        me:state(STATE.NORMAL)
        me:hp(tonumber(hp))
        report(me, true, step)
    end,

    verify_alive_hp = function(me, step, hp)
        local state = me:state()
        local now = me:hp()
        me:state(STATE.NORMAL)
        me:hp(me:maxhp())
        if state ~= STATE.NORMAL or now ~= tonumber(hp) then
            return report(me, false, step, string.format('state=%d hp=%d expected=%s', state, now, tostring(hp)))
        end
        report(me, true, step)
    end,

    verify_clock_hour = function(me, step, hour)
        local t = datetime()
        if t.hour == tonumber(hour) then
            report(me, true, step)
        else
            report(me, false, step, string.format('hour=%d expected=%s', t.hour, tostring(hour)))
        end
    end,

    verify_achievement_replaced = function(me, step, id)
        id = tonumber(id)
        me:push_achievement(id, 'old', 1, 1)
        local held = me:achievement(id)
        me:push_achievement(id, 'new', 1, 1)
        local text = held ~= nil and held:text() or nil
        me:erase_achievement(id)
        if text == nil then
            report(me, true, step)
        else
            report(me, false, step, 'a replaced achievement still answered ' .. tostring(text))
        end
    end,

    verify_lock_guard = function(me, step, kind, ms)
        local group = kind == 'group' and me:group() or nil
        local clan = kind == 'clan' and me:clan() or nil
        if (group == nil and clan == nil) or me:hold_lock(kind, tonumber(ms)) ~= true then
            return report(me, false, step, 'could not hold the ' .. kind .. ' lock')
        end

        local names = {}
        local money = nil
        if group ~= nil then
            for _, name in pairs(group:members()) do
                table.insert(names, name)
            end
        else
            money = clan:money()
            for _, member in pairs(clan:members()) do
                table.insert(names, member.name)
            end
        end

        for _, name in ipairs(names) do
            if name == '__lock__' then
                return report(me, false, step, 'read the ' .. kind .. ' members while the guard was held')
            end
        end
        if money == 777 then
            return report(me, false, step, 'read the clan money while the guard was held')
        end
        report(me, true, step)
    end,

    clan_money = function(me, step)
        local clan = me:clan()
        if clan == nil then
            return report(me, false, step, 'no clan')
        end
        report(me, true, step, tostring(clan:money()))
    end,

    -- test.reward.split pays 3 가시철단도, one per slot. test.reward.over pays 250 도토리, more than one stack holds.
    reward_split = function(me, step)
        local split = me:reward('test.reward.split')
        local over = me:reward('test.reward.over')
        local counts = {}
        for _, name in ipairs({ '가시철단도', '도토리' }) do
            counts[name] = 0
            for _, item in pairs(me:items(name) or {}) do
                counts[name] = counts[name] + item:count()
            end
        end
        local paid = split == true and counts['가시철단도'] == 3 and over ~= true and counts['도토리'] == 0
        report(me, paid, step,
            string.format('split=%s over=%s 가시철단도=%d 도토리=%d', tostring(split), tostring(over), counts['가시철단도'],
                counts['도토리']))
    end,

    madong_prepare = function(me, step)
        me:promotion(3)
        me:remove_quest(require('lib.quest').QUEST_MADONG_APPLY)
        report(me, true, step)
    end,

    -- expected: the 'class|partner|schedule' param of the madong apply quest.
    verify_madong_param = function(me, step, expected)
        local q = me:quest(require('lib.quest').QUEST_MADONG_APPLY)
        local actual = q and q:param() or 'none'
        report(me, actual == expected, step, tostring(actual))
    end,

    -- order: 'kill_first' or 'heal_first' for the buff order. 노도성황 kills at 1 hp, 운기 restores mp each tick.
    concast_after_death = function(me, step, order)
        me:unbuff('노도성황')
        me:unbuff('운기')
        if order == 'kill_first' then
            me:buff('노도성황', 30)
            me:buff('운기', 11)
        else
            me:buff('운기', 11)
            me:buff('노도성황', 30)
        end
        me:mp(0)
        me:hp(1)
        sleep(2500)

        local ghost = me:state() == STATE.GHOST
        local mp = me:mp()
        me:unbuff('노도성황')
        me:unbuff('운기')
        me:state(STATE.NORMAL)
        me:hp(me:maxhp())
        me:mp(me:maxmp())
        report(me, ghost, step, string.format('mp=%d', mp))
    end,

    drop_escrow_items = function(me, step)
        me:drop_escrow_items()
        report(me, true, step)
    end,

    -- The money free space counts held money, so it shows whether any escrow money is left.
    verify_money_released = function(me, step, expected)
        local money = me:money()
        local free = me:money_free_space()
        local ok = money == tonumber(expected) and free == 0xFFFFFFFF - money
        report(me, ok, step, string.format('money=%d free=%d', money, free))
    end,

    -- Below level 5 the 낙랑의두루마리2 reward includes 50 exp.
    nakrang2_prepare = function(me, step, level)
        local quest = require('lib.quest')
        me:level(tonumber(level))
        me:remove_quest(quest.QUEST_NAKRANG2)
        local q = me:start_quest(quest.QUEST_NAKRANG2)
        if q ~= nil then
            q:step(1)
            q:progress(2)
        end
        local scroll = me:mkitem('낙랑의두루마리2', 1)
        report(me, q ~= nil and scroll ~= nil, step, string.format('exp=%d', me:exp()))
    end,

    verify_nakrang2 = function(me, step, completed, exp)
        local q = me:quest(require('lib.quest').QUEST_NAKRANG2)
        local done = q ~= nil and q:completed()
        local rewarded = me:item('낙랑의두루마리3') ~= nil
        local ok = done == (completed == 'true') and rewarded == done and me:exp() == tonumber(exp)
        report(me, ok, step,
            string.format('completed=%s rewarded=%s exp=%d', tostring(done), tostring(rewarded), me:exp()))
    end,

    -- transfer_to throws a C++ exception when the destination table is missing.
    builtin_exception = function(me, step)
        local ok, err = pcall(function()
            return me:transfer_to('127.0.0.1', 1)
        end)
        local raised = ok == false and tostring(err):find('requires a destination table', 1, true) ~= nil
        report(me, raised, step, tostring(err))
    end,

    -- Takes count pieces by name the way an NPC sale or a quest does, even while trading.
    remove_items = function(me, step, name, count)
        local removed = me:rmitem(name, tonumber(count))
        report(me, removed ~= nil and removed ~= false, step, tostring(removed))
    end,

    -- slot: 1-based inventory slot. Drops the whole stack even while trading, unlike the client drop.
    drop_slot = function(me, step, slot)
        local dropped = me:dropitem(tonumber(slot), false)
        report(me, dropped ~= nil, step)
    end,

    -- cost, reward: comma-separated 'name=count' pairs, 'money=n' for money, '-' for nothing.
    -- expected: an exchange_result name, e.g. 'OK' or 'LACK_CAPACITY'.
    exchange_items = function(me, step, cost, reward, expected)
        local specs = {}
        for i, spec in ipairs({ cost, reward }) do
            specs[i] = { ['item'] = {} }
            for name, count in tostring(spec):gmatch('([^,=]+)=(%d+)') do
                if name == 'money' then
                    specs[i]['money'] = tonumber(count)
                else
                    specs[i]['item'][name] = tonumber(count)
                end
            end
        end

        local code = me:exchange(specs[1], specs[2])
        local actual = 'UNKNOWN'
        for name, value in pairs(require('lib.enum').exchange_result) do
            if value == code then
                actual = name
            end
        end

        if actual == expected then
            report(me, true, step)
        else
            report(me, false, step, string.format('exchange is %s, expected %s', actual, tostring(expected)))
        end
    end,

    store_item = function(me, step, name, count)
        local _, item = next(me:items(name) or {})
        if item == nil then
            return report(me, false, step, 'no held ' .. tostring(name))
        end
        if me:store_item(item, tonumber(count)) ~= true then
            return report(me, false, step, string.format('cannot store %s=%s', name, tostring(count)))
        end
        report(me, true, step)
    end,

    -- expected: 'ok' when the retrieve must succeed, 'fail' when it must be rejected.
    retrieve_item = function(me, step, name, count, expected)
        local stored = me:stored_item(name)
        if stored == nil then
            return report(me, false, step, 'no stored ' .. tostring(name))
        end
        local actual = me:retrieve_item(stored, tonumber(count)) ~= nil and 'ok' or 'fail'
        if actual == expected then
            report(me, true, step)
        else
            report(me, false, step,
                string.format('retrieve %s=%s is %s, expected %s', name, tostring(count), actual, tostring(expected)))
        end
    end,

    drain_stored = function(me, step, name, piece)
        piece = tonumber(piece)
        local stored = me:stored_item(name)
        while stored ~= nil do
            local count = math.min(stored:count(), piece)
            if me:retrieve_item(stored, count) == nil or me:rmitem({ [name] = count }) ~= true then
                return report(me, false, step, 'cannot drain stored ' .. tostring(name))
            end
            stored = me:stored_item(name)
        end
        report(me, true, step)
    end,

    verify_stored = function(me, step, name, expected)
        local actual = 0
        for _, item in pairs(me:stored_item() or {}) do
            if item:model():name() == name then
                actual = actual + item:count()
            end
        end
        if actual == tonumber(expected) then
            report(me, true, step)
        else
            report(me, false, step, string.format('stored %s=%d(expected %s)', name, actual, tostring(expected)))
        end
    end,

    -- Wears, holds and stores the clan sword of a castle the character does not own, fills the
    -- inventory, then strips it. Every copy must be gone even though the inventory has no room.
    verify_castle_strip = function(me, step)
        local castle_lib = require('lib.castle')
        local clan = me:clan()
        local divine_beast = nil
        for beast, _ in pairs(castle_lib.WEAPON_NAME_BY_DIVINE_BEAST) do
            local entity = castle(beast)
            if entity ~= nil and (clan == nil or entity:owner_clan() ~= clan:id()) then
                divine_beast = beast
                break
            end
        end
        if divine_beast == nil then
            return report(me, false, step, 'no loaded castle the clan does not own')
        end
        local name = castle_lib.WEAPON_NAME_BY_DIVINE_BEAST[divine_beast]

        local level = me:level()
        if level < 30 then
            me:level(30)
        end

        local worn = me:mkitem(name, 1)
        if worn ~= nil then
            me:active(worn)
        end
        local weapon = me:weapon()
        if weapon == nil or weapon:model():name() ~= name then
            me:level(level)
            return report(me, false, step, 'cannot wear ' .. name)
        end

        local stored = me:mkitem(name, 1)
        if stored == nil or me:store_item(stored, 1) ~= true then
            me:level(level)
            return report(me, false, step, 'cannot store ' .. name)
        end
        if me:mkitem(name, 1) == nil then
            me:level(level)
            return report(me, false, step, 'cannot hold ' .. name)
        end

        local filled = 0
        for _ = 1, 52 do
            if me:mkitem('목도', 1) == nil then
                break
            end
            filled = filled + 1
        end

        castle_lib.strip(me, divine_beast)

        local left = {}
        for _, item in pairs(me:equipments()) do
            if item:model():name() == name then
                table.insert(left, 'worn')
            end
        end
        if #me:items(name) > 0 then
            table.insert(left, 'held')
        end
        for _, item in pairs(me:stored_item()) do
            if item:model():name() == name then
                table.insert(left, 'stored')
            end
        end

        if filled > 0 then
            me:rmitem({ ['목도'] = filled })
        end
        me:level(level)

        if #left == 0 then
            report(me, true, step)
        else
            report(me, false, step, 'left ' .. table.concat(left, ','))
        end
    end,

    -- Reports 'q=<id>:<state>,...;m=<money>;map=<id>;i=<name>=<count>,...' for the NPC explorer.
    -- Quest state is 'none', '<step>' or '<step>!' when completed. Items are summed by name and sorted.
    snapshot = function(me, step, quest_ids)
        report(me, true, step, explorer_snapshot(me, quest_ids))
    end,

    -- Applies a set_state spec; see apply_state for the format.
    set_state = function(me, step, spec)
        local error = apply_state(me, spec)
        if error ~= nil then
            return report(me, false, step, error)
        end
        report(me, true, step)
    end,

    -- Reports the snapshot, then applies the set_state spec so the explorer saves one request
    -- per dialog path.
    snapshot_reset = function(me, step, quest_ids, spec)
        local text = explorer_snapshot(me, quest_ids)
        local error = apply_state(me, spec)
        if error ~= nil then
            return report(me, false, step, error)
        end
        report(me, true, step, text)
    end,

    make_items = function(me, step, name, count, seconds)
        local item = me:mkitem(name, tonumber(count), true, tonumber(seconds))
        report(me, item ~= nil, step)
    end,

    verify_unknown_item_name = function(me, step, name)
        local function count_of()
            local total = 0
            for _, item in pairs(me:items(name) or {}) do
                total = total + item:count()
            end
            return total
        end

        local unknown = 'integration_unknown_item'
        local before = count_of()
        local made = me:mkitem({ [name] = 1, [unknown] = 1 })
        local removed = me:rmitem({ [name] = 1, [unknown] = 1 })
        local after = count_of()

        if made ~= nil then
            report(me, false, step, 'mkitem made part of a table with an unknown name')
        elseif removed ~= false then
            report(me, false, step, 'rmitem took part of a table with an unknown name')
        elseif after ~= before then
            report(me, false, step, string.format('%s count changed %d -> %d', name, before, after))
        else
            report(me, true, step)
        end
    end,

    verify_state_machine_objects = function(me, step)
        local sm = state_machine('integration_probe')
        if sm == nil then
            return report(me, false, step, 'integration_probe machine is missing')
        end

        local direct = pcall(function()
            sm:call('on_probe_peek', me)
        end)
        local nested = pcall(function()
            sm:call('on_probe_peek', { owner = me })
        end)
        if direct or nested then
            report(me, false, step,
                string.format('object accepted direct=%s nested=%s', tostring(direct), tostring(nested)))
        else
            report(me, true, step)
        end
    end,

    verify_no_global_args = function(me, step)
        report(me, args == nil, step, tostring(args))
    end,

    cleanup = function(me, step)
        local map = me:map()
        if map ~= nil then
            for _, obj in pairs(map:objects(OBJECT_TYPE.MOB)) do
                obj:destroy()
            end
        end
        report(me, true, step or 'cleanup')
    end
}
