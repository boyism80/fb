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
            report(me, false, step, string.format('quest %s is %s, expected %s', tostring(id), actual, tostring(expected)))
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
