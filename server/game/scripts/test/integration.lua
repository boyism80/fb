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

-- Links two parts to a body and sets its parts mode.
-- mode: 'PARTS' (default) or 'BODY'.
function link_parts(me, step, body_oid, p1_oid, p2_oid, mode)
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
end

-- PARTS: hitting a part damages the part and forwards to the body.
-- BODY:  hitting a part damages only the body; part HP is unchanged.
function verify_parts_hit(me, step, body_oid, part_oid, mode)
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
end

-- Body with parts ignores direct hits.
function verify_body_direct_ignored(me, step, body_oid)
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
end

-- PARTS death: kill p1 only → body survives; kill p2 → body dies.
-- BODY death: damage through part until body dies; part never dies alone first.
function verify_parts_death(me, step, body_oid, p1_oid, p2_oid, mode)
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
end

-- PHYSICAL / MAGIC damage resist (C++ calculate_damage path, fixed=false).
-- value '1.0' → HP unchanged; '0.0' → HP decreased.
function verify_damage_resist(me, step, oid, resist_type, value)
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
end

-- SPELL gate: with SPELL=1.0 magic skill damage must not apply.
-- Mimics spell.damage's resisted(SPELL) check before damage_to.
function verify_spell_gate(me, step, oid, value)
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
end

-- Cross-type: PHYSICAL resist must not affect magic damage and vice versa.
function verify_cross_type(me, step, oid)
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
end

function verify_gate(me, step, resist_type, value)
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
end

function verify_clamp(me, step)
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
end

function verify_link_reject(me, step, case)
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
end

function cleanup(me, step)
    local map = me:map()
    if map ~= nil then
        for _, obj in pairs(map:objects(OBJECT_TYPE.MOB)) do
            obj:destroy()
        end
    end
    report(me, true, step or 'cleanup')
end
